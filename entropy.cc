#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/static_assert.hpp>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static const size_t kBlockSize = 65536;

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
		float p_x = float(histogram[i]) / kBlockSize;
		if (p_x > 0) {
			h += -p_x * logf(p_x) * ONE_OVER_LOG2;
		}
	}
	
	return h;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: %s <device>\n", argv[0]);
		return 1;
	}
	
	const char * const device = argv[1];
	
	int fd = open(device, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	
	uint8_t block[kBlockSize];
	ssize_t bytes;
	while ((bytes = read(fd, block, kBlockSize)) == (ssize_t)kBlockSize) {
		float h = entropy(block);
		printf("%.04f\n", h);
	}
	
	if (bytes < 0) {
		perror("read");
		return 1;
	} else if (bytes == 0) {
		fprintf(stderr, "EOF\n");
	} else {
		fprintf(stderr, "read: returned %d\n", bytes);
		return 1;
	}
	
	close(fd);
	return 0;
}
