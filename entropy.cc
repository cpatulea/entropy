#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <boost/static_assert.hpp>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static const size_t kBlockSize = 65536;
static uint8_t block[kBlockSize];

static float entropy(const uint8_t block[kBlockSize]) {
	uint16_t histogram[256];
	
	// Ensure no histogram overflow.
	BOOST_STATIC_ASSERT((1ULL << (8 * sizeof(histogram[0]))) >= kBlockSize);

	// Compute block histogram.
	memset(histogram, 0, sizeof(histogram));
	for (size_t i = 0; i < kBlockSize; i++) {
		histogram[block[i]]++;
	}
	
	// Compute block entropy.
	static const float ONE_OVER_LOG2 = 1.f / logf(2.f);
	float h = 0.f;
	for (size_t i = 0; i < ARRAY_SIZE(histogram); i++) {
		if (histogram[i]) {
			float p_x = float(histogram[i]) / kBlockSize;
			h += -p_x * logf(p_x) * ONE_OVER_LOG2;
		}
	}
	
	return h;
}

int main(int argc, char **argv) {
	off64_t offset = 0;
	
	if (argc == 3) {
		offset = strtoull(argv[2], NULL, 0);
	} else if (argc != 2) {
		printf("usage: %s <device> [<start offset>]\n", argv[0]);
		return 1;
	}
	
	const char * const device = argv[1];
	
	int fd = open(device, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	
	if (offset) {
		if (lseek64(fd, offset, SEEK_SET) != offset) {
			perror("lseek64");
			return 1;
		}
	}

	for (;;) {
		ssize_t bytes;
		float h;

		bytes = read(fd, block, kBlockSize);
		offset += kBlockSize;

		if (bytes < 0) {
			if (errno == EIO) {
				printf("NaN\n");

				// Try to skip bad sectors.
				if (lseek64(fd, offset, SEEK_SET) != offset) {
					perror("lseek64");
					return 1;
				}
				
				continue;
			} else {
				perror("read");
				return 1;
			}
		} else if (bytes == 0) {
			break;
		} else if (bytes != kBlockSize) {
			fprintf(stderr, "warning: discarded trailing %d bytes\n", bytes);
			continue;
		}
		
		h = entropy(block);
		printf("%.04f\n", h);
	}
	
	close(fd);
	return 0;
}
