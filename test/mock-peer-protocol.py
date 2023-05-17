import socket
import sys
import selectors
import fcntl
import os
import struct
import math

HOST = None
PORT = 50007
s = None
for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC,
                              socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
    af, socktype, proto, canonname, sa = res
    try:
        s = socket.socket(af, socktype, proto)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    except OSError as msg:
        s = None
        continue
    try:
        s.bind(sa)
        s.listen(1)

        print(f'listen {s.getsockname()}')
    except OSError as msg:
        s.close()
        s = None
        continue
    break
if s is None:
    print('could not open socket')
    sys.exit(1)

conn, addr = s.accept()
test_file = open('./test-file.png', 'rb')
test_file_size = os.path.getsize('./test-file.png')

PIECE_SIZE = 1 << 15
BLOCK_SIZE = 1<<14

pieces_in_file = math.ceil(test_file_size/PIECE_SIZE)
blocks_per_piece = math.ceil(PIECE_SIZE / BLOCK_SIZE)

print(f'pieces in file: {pieces_in_file}\nblocks per piece{blocks_per_piece}')

def read_block(piece_index, block_offset):
    piece_offset = piece_index * PIECE_SIZE
    byte_offset = piece_offset + block_offset
    block_len = min(test_file_size - byte_offset, BLOCK_SIZE)

    test_file.seek(byte_offset)
    return test_file.read(block_len)  

def cmd(stdin, mask):
    cmd = stdin.read().rstrip()
    print('Send cmd', cmd)
    
    len_field = 0
    id_field = None

    if cmd == 'keep-alive':
        data = (chr(0) * 4).encode()
        conn.sendall(data)
    elif cmd == 'choke':
        data = struct.pack('!IB', 1, 0)
        conn.sendall(data)
    elif cmd == 'unchoke':
        data = struct.pack('!IB', 1, 1)
        conn.sendall(data)
    elif cmd == 'interested':
        data = struct.pack('!IB', 1, 2)
        conn.sendall(data)
    elif cmd == 'not-interested':
        data = struct.pack('!IB', 1, 3)
        conn.sendall(data)
    elif cmd == 'have':
        data = struct.pack('!IBI', 5, 4, 10)
        conn.sendall(data)
    elif cmd == 'bitfield':
        data = struct.pack('!IBBB', 3, 5, 0b11010011, 0b00111000)
        conn.sendall(data)
    elif cmd == 'request':
        data = struct.pack('!IBIII', 13, 6, 2, 1024 * 2, 1024)
        conn.sendall(data)
    elif cmd == 'piece':
        block = ''
        for i in range(1024):
            block = block + chr(i%256)
        block = block.encode()

        data = struct.pack('!IBII1024s', 9 + 1024, 7, 2, 2*1024, block)
        print('packet len', len(data))
        conn.sendall(data)
    elif cmd == 'cancel':
        data = struct.pack('!IBIII', 13, 8, 2, 1024 * 2, 1024)
        conn.sendall(data)
    elif cmd == 'send-test-file':
        print('send')
        for piece_i in range(pieces_in_file):
            for block_i in range(blocks_per_piece):
                block = read_block(piece_i, block_i * BLOCK_SIZE)
                offset = piece_i * PIECE_SIZE + block_i * BLOCK_SIZE
            
                data = struct.pack(f'!IBII{len(block)}s', 9 + BLOCK_SIZE, 7, piece_i, block_i*BLOCK_SIZE, block)
                conn.sendall(data)
            

    elif cmd == 'get-test-file':
        out_test_file = open('./test-out.png', 'wb')
        for piece_i in range(pieces_in_file):
            print(f'start piece {piece_i}')
            for block_i in range(blocks_per_piece):
                print(f'start block {block_i}')
                offset = piece_i * PIECE_SIZE + block_i * BLOCK_SIZE
                if offset < 0:
                    continue
                block_len = min(test_file_size - offset, BLOCK_SIZE)
                expected_response_bytes = 13 + block_len
                data = struct.pack('!IBIII', 13, 6, piece_i, block_i * BLOCK_SIZE, block_len)
                conn.sendall(data)
                print('sent')

                while True:
                    try:
                        got_bytes = 0
                        buffer = b''
                        while got_bytes != expected_response_bytes:
                            bytes = conn.recv(BLOCK_SIZE)
                            buffer = buffer + bytes
                            got_bytes += len(bytes)
                        print(f'recvd {len(buffer)}')
                        (_, _, _, _, block) = struct.unpack(f'!IBII{block_len}s', buffer)
                        
                        '''
                        print('block', end=' ')
                        for byte in block:
                            print('{0:04x}'.format(int(byte)), end=' ')
                        print()
                        '''

                        out_test_file.seek(offset)
                        print('after seek')
                        out_test_file.write(block)
                        print('should break')
                        break
                    except BlockingIOError:
                        continue

                print(f'block {block_i}')
            print(f'piece {piece_i}')

        out_test_file.close()
        print('done')
def recv(conn, mask):
    data = conn.recv(4)

    if not data:
        sel.unregister(conn)
        return

    print('got message')

    pack_len = struct.unpack("!I", data)[0]
    print('len', pack_len)
    if pack_len == 0:
        return

    data = conn.recv(pack_len)
    print('data', data)

with conn:
    print('Connected by', addr)
    pstr = "BitTorrent protocol"

    handshake = conn.recv(49 + len(pstr))

    pstrlen = chr(len(pstr)).encode()
    reserved = (chr(0) * 8).encode()

    hash_offset = 1 + len(pstr) + 8
    info_hash = handshake[hash_offset:hash_offset+20]
    pid = (chr(12) * 20).encode()

    print('handshake', handshake)
    print('pstrlen', pstrlen)
    print('pstr', pstr.encode('ASCII'))
    print('reserved', reserved)
    print('info_hash', info_hash)
    print('pid', pid)
    print('package:', pstrlen + pstr.encode('ASCII') + reserved + info_hash + pid)
    my_handshake = pstrlen + pstr.encode('ASCII') + reserved + info_hash + pid
    conn.sendall(my_handshake)

    conn.setblocking(False)
    orig_fl = fcntl.fcntl(sys.stdin, fcntl.F_GETFL)
    fcntl.fcntl(sys.stdin, fcntl.F_SETFL, orig_fl | os.O_NONBLOCK)

    sel = selectors.DefaultSelector()
    sel.register(conn, selectors.EVENT_READ, recv)
    sel.register(sys.stdin, selectors.EVENT_READ, cmd)

    while True:
        events = sel.select()
        for key, mask in events:
            callback = key.data
            callback(key.fileobj, mask)