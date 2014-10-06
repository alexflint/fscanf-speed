**How fast is fscanf?**

Suppose you wish to read a large text file containing N floating point
numbers on each line. Such a file that might have been created, for
example, by `numpy.savetxt` in python.

What is the fastest way to read such a file? In python, one option is
to use `numpy.loadtxt`. Unfortunately, this is quite slow. For a file
with one million rows and five columns:

```bash
$ time python -c 'import numpy; numpy.loadtxt("large.txt")'

real    0m7.037s
user    0m6.800s
sys     0m0.230s
```

I decided to find out much of this was I/O bound and how much was
consumed by actual processing, so I wrote a simple C++ program that
uses `fscanf` to read the five floating point numbers. The critical
loops is very simple:

```c++
	FILE* fd = fopen(path, "r");
 	while (!feof(fd)) {
		fscanf(fd, "%lf %lf %lf %lf %lf\n",
					 &line[0], &line[1], &line[2], &line[3], &line[4]);
	}
	fclose(fd);
```

Here are the results of parsing the same file (one million rows, five columns):

```bash
$ c++ -O3 --std=c++11 -o loadtxt loadtxt.cpp
$ time ./loadtxt --fscanf large.txt

real    0m2.712s
user    0m2.673s
sys	    0m0.038s
```

As you can see, it is just shy of a three times as fast as
`numpy.loadtxt`. This is of course much less general than
`numpy.loadtxt`, not least because the number of columns is
hard-coded.

Now for the surprising part. I imagined that since fscanf has been a
part of libc for so very long, it would be about as fast as it is
possible to be. Certainly I imagined that the C loop above would be
almost entirely I/O bound. But I was wrong.

The first give-away was when I tried using `pandas.load_csv` from
python:

```bash
time python -c 'import pandas; pandas.read_csv("large.txt", sep=" ", header=None)'

real    0m1.086s
user    0m0.930s
sys     0m0.149s
```

This gives nearly a _three times_ speedup over the fscanf loop, even
including the overhead of loading up python and importing `pandas`! At
this point I became curious about how quickly a lean C program could
parse the text file if instead of using `fscanf` I used a custom
parser ascii-to-floating-point parser. Here are the results:

```bash
$ time ./loadtxt --direct large.txt

real    0m0.320s
user    0m0.287s
sys     0m0.032s
```

That is a _further 3x_ speedup over pandas! At this point we are
almost 10 times faster than the `fscanf` loop and a full 30 times
faster than `numpy.loadtxt`.
