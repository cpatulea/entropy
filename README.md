# entropy
entropy is a small utility that allows you to efficiently calculate the entropy of blocks in a block device (or file). It is geared toward determining the extent of single-disk data corruption in RAID-5 arrays.

Note that in my case my array was already in degraded mode (one missing drive) so I was not able to take advantage of RAID-5's inherent data recovery.

# RAID-5 Data Layout
[RAID-5 arrays][wiki-raid5] use parity to recover from single-disk failures. In a 5-disk array, the filesystem and parity data are laid out as follows (from [Linux RAID-5 Algorithms][linux-raid5]):

![Data layout for left-synchronous aka left-symmetric RAID-5](http://www.accs.com/p_and_p/RAID/images/raid5LS.gif)

Each row is referred to as a "stripe" and has a size determined at array creation time, usually around 64 K.

# Finding Damaged Areas
When some unknown area of one of the drives is overwritten, we can use this layout to our advantage because corresponding blocks within one stripe will contain similar data. By detecting areas that are dissimilar in one drive compared to the others, we can find the overwritten area.

One measure of similarily is [entropy][wiki-entropy]. In this context, we can think of entropy as the amount of information or randomness in a particular block of data. For example, English text files have low entropy (English alphabet out of 256 possible byte value) compared to MP3 files (compressed to maximize the amount of information per byte).

Entropy also has some useful practical properties:

* **compression**: when working with 750 GB drives, storing anything more than a few bytes per stripe is space-prohibitive
* **simple**: entropy is relatively easy to calculate and "hard to get wrong"
* **efficient**: for small alphabets (2<sup>8</sup>) and comparatively large data blocks (64 K), calculating entropy can be done with one pass over the block

# Results
This is the result of plotting the output of the utility on two drives, one good and one damaged, from the same RAID-5 array:

![Plot comparing stripe entropy from a damaged drive to a good drive from the same RAID array](../../raw/master/entropy-example-full.png)

This shows that most of the drive is still intact, since the entropy on the damaged drive closely matches the entropy on the other. In my case, I knew that the area was somewhere near the end of the drive, so by zooming in to the far right of the plot, we can see where they start to differ:

![Plot zoomed in on the end of the drives](../../raw/master/entropy-example-tail.png)

Here, we can see that the drives match up to the ~80<sup>th</sup> block from the end. The damaged drive contained a sequence of "12 34" while the good drive contained only "00"s. (I got lucky where was no real data there!)

The only exception to that was the small spike at the end of the good drive (last data point). That is the md superblock, which contains some metadata for automatic RAID-5 array assembly at boot time. Luckily that data can be reconstructed fairly easily by looking at the superblock on the other array members.

Once I zero'ed out the area and reconstructed the superblock, the array came up fine. Phew!

# Conclusion
This utility allows you to efficiently compare the contents of two disks (or very large files). This helps to pinpoint corrupted areas and, if the data from the good drive gives any hints to what was on the damaged drive, we can manually perform a recovery on the affected areas.

[wiki-raid5]: http://en.wikipedia.org/wiki/Standard_RAID_levels#RAID_5
[linux-raid5]: http://www.accs.com/p_and_p/RAID/LinuxRAID.html
[wiki-entropy]: http://en.wikipedia.org/wiki/Entropy_(information_theory)
