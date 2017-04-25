# Bitstream Matching Module

## What is it

Match two TS bitstream and translate codeword byte-wise.
It's for Backscatter-DVBT system.

## Usage

1. gcc -O3 -o bin/bit_matching bit_matching.c
2. Generate Raw and Backscattered files
3. ./bin/bit_matching file_raw file_data
4. Voila!

## To-Do

1. Better matching algorithm
2. 4M-Byte-Wise rather than Byte-wise
3. Improve robustness by implementing error bit tolerence

## Team Members

- Qiuyue Xue
- Caihua Li
- Lei Yang
- Ziyang Xu
