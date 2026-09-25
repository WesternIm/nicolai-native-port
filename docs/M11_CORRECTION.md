# M11 correction made in M12

M11 correctly reverse-engineered the generic SpeechCube `cmp16sbi` entropy
front-end, but incorrectly assumed that Nicolai's ANA stream used that codec.

M12 proved that Nicolai's acoustic descriptor has signal-coding type `2`, while
the x86 constructor enables cmp16 only for type `0x50`.  A separate legacy enum
mapping identifies type `2` as `loi A` (G.711 A-law), type `1` as `loi mu`, and
type `3` as linear.

Therefore:

- keep the M11 cmp16/Huffman implementation as valid generic SpeechCube work;
- do **not** route Nicolai through it;
- Nicolai ANA bytes decode directly, one byte to one PCM16 sample, using G.711
  A-law expansion.
