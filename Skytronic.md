This are my notes on the Skytronic remote 350.221:

All high pulses in the burst are relativly equally short. So apparantly it's not Manchesterencoded, as it would also have longer high pulses. The burst does have shorter and longer pulses.
The sum of a high and low pulse are: 63 or 93

It exists out of 68 pulses.

It seems that the pulses are constructed by mixing two pulses, as the high-pulses seem to shrink further in the burst (28 downto 22). Also the low pulses expand (35 -> 39).

[28](.md)   35 [28](.md)   35 [28](.md)   36 [27](.md)   36 [27](.md)   66 [27](.md)   37 [27](.md)   36
[26](.md)   68 [26](.md)   66 [26](.md)   39 [25](.md)   37 [25](.md)   69 [25](.md)   38 [24](.md)   71
[23](.md)   38 [25](.md)   69 [24](.md)   68 [25](.md)   38 [25](.md)   39 [25](.md)   70 [23](.md)   38
[25](.md)   71 [22](.md)   70 [22](.md)   41 [23](.md)   71 [22](.md)   40 [24](.md)   69 [23](.md)   40
[24](.md)   70 [22](.md)   41 [24](.md)   39 [24](.md)   71 [22](.md)   70 [22](.md)   41 [23](.md)   41
[22](.md)   72 [22](.md)   70 [23](.md)   39 [24](.md)   71 [21](.md)   41 [23](.md)   71 [22](.md)   40
[24](.md)   39 [24](.md)   71 [22](.md)   40 [23](.md)   72 [22](.md)   71 [21](.md)   41 [23](.md)   41
[23](.md)   71 [22](.md)   40 [22](.md)   72 [22](.md)   40 [23](.md)   73 [21](.md)   40 [22](.md)   73
[21](.md)   40 [23](.md)   72 [22](.md)   39 [23](.md)   73 [21](.md)   40 [23](.md)   71 [22](.md)   40
[23](.md)   72 [21](.md)   40 [23](.md)   72 [22](.md)   39 [24](.md)   71 [22](.md)  642
Manchester 68 93 3 5261

Short Long Short Short equal to an encoded 1 bit
Short Short Short Long equal to an encoded 0 bit

#define	ShortHighPulseDuration\_Min 22
#define	ShortHighPulseDuration\_Max 36

#define	ShortLowPulseDuration\_Min 29
#define	ShortLowPulseDuration\_Max 42

#define LongLowPulseDuration\_Min 59
#define	LongLowPulseDuration\_Max 72

#define	TerminatorDuration\_Min 630
#define	TerminatorDuration\_Max 720