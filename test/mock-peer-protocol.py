import socket
import sys
import selectors
import fcntl
import os
import struct

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