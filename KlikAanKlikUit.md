These are my notes on the KAKU-protocol:

When i press a button on a TMT-502 remote, i receive the following burst of pulses:
[20](.md)   68 [20](.md)   68 [19](.md)   68 [64](.md)   25 [18](.md)   69 [19](.md)   69
[19](.md)   69 [19](.md)   68 [19](.md)   69 [19](.md)   69 [19](.md)   69 [19](.md)   68
[19](.md)   69 [19](.md)   69 [19](.md)   69 [19](.md)   68 [19](.md)   69 [19](.md)   69
[19](.md)   69 [62](.md)   26 [18](.md)   69 [63](.md)   25 [18](.md)   70 [62](.md)   26 [18](.md)  623

My first observation is that the sum of the high and low pulselengths are almost the same:
20 + 68 = 88
19 + 68 = 87
64 + 25 = 89
19 + 69 = 88

I think the decoding of the burst of pulses are as follows. An entire encoded bit has a length of 88. When the transition is in the first halve of the encoded bit, it is an encoded 0. When the transition is in the seconde halve of the encoded bit, it is an encoded 1.

[20](20.md) 68 -> has a transition in the first halve, since the high pulse is shorter than 44
[62](62.md) 26 -> has a transition in the second halve, since the high pulse is longer than 44