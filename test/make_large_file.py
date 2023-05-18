from random import randint
import sys
import os

if len(sys.argv) != 3:
	print(f'usage: {sys.argv[0]} filename num_bytes')
	sys.exit(1)

filename = sys.argv[1]
size = int(sys.argv[2])

block_size = 1024;
with open(filename, 'wb') as f:
	for i in range(size//block_size):
		f.write(os.urandom(block_size))

