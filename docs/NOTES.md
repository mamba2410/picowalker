# Notes

## Walk start

Sometimes it seems to decompress wrong?
togepi on noisy forest broke.
I'd never sent togepi before.
Seems to have zeroed from ADDR_ROUTE_INFO to halfway through TEST_ROUTE_NAME.
This is (0x907e+160 - 0x8f00)=0x21e(542) bytes
Equates to 0xd700-0xd91e

Discovered that sometimes the copy-read from 0xd700 reads all zeros, but the
next debug read from 0xd700 reads fine?
So yeah I have no idea.

