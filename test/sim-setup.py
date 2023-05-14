import os
import subprocess

def copy_shared_libs(node, paths):
	for path in paths:
		print(f'copy {path}')
		to_path = path
		
		if to_path[-1] == '*':
			to_path = to_path[:-1]
		
		subprocess.run(f'hcp {path} {node}:{to_path}', shell=True)
	
	lib_dirs = set()
	for path in paths:
		i = path.rfind('/')
		lib_dirs.add(path[:i+1])
	
	for d in lib_dirs:
		subprocess.run(f'himage {node} ldconfig -n {d}', shell=True)
	
# go to top level project dir
os.chdir('../..')

# setup tracker
print('tracker setup start')
subprocess.run('hcp opentracker/opentracker.debug tracker:/opentracker', shell=True)
print('tracker setup done')

torrent_file_path = './ftorrent/test/test-file.torrent'

# setup client
print('client setup start')
subprocess.run('hcp ./ftorrent/build/demo/demo client:/ftorrent', shell=True)
libs = [
	'/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.28',
	'/usr/lib/liburing.so.2.4',
	'/usr/lib/liburing-ffi.so.2.4',
	'/usr/lib/liburing.a',
	'/usr/lib/liburing-ffi.a',
	'/usr/local/lib/*',
	'/usr/lib/x86_64-linux-gnu/*'
]
copy_shared_libs('client', libs)
subprocess.run(f'hcp {torrent_file_path} client:/', shell=True)
print('client setup done')

# setup peers
num_peers = 2
peers = [f'peer{i + 1}' for i in range(num_peers)]
libs = [
	'/usr/lib/x86_64-linux-gnu/libcurl.so.4.6.0',
	'/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.28',
	'/usr/lib/x86_64-linux-gnu/libevent-2.1.so.7.0.0',
	'/usr/lib/x86_64-linux-gnu/libevent_core-2.1.so.7.0.0',
	'/usr/lib/x86_64-linux-gnu/libevent_extra-2.1.so.7.0.0',
	'/usr/lib/x86_64-linux-gnu/libevent_openssl-2.1.so.7.0.0',
	'/usr/lib/x86_64-linux-gnu/libevent_pthreads-2.1.so.7.0.0',
	'/usr/local/lib/libssl.so.53.0.1',
	'/usr/local/lib/libcrypto.so.50.0.1',
	'/usr/lib/x86_64-linux-gnu/libminiupnpc.so.17',
	'/usr/lib/x86_64-linux-gnu/libssl.so.1.1',
	'/usr/lib/x86_64-linux-gnu/libssh.so.4.8.4',
	'/usr/lib/x86_64-linux-gnu/libbrotlidec.so.1.0.7',
	'/usr/lib/x86_64-linux-gnu/libcrypto.so.1.1',
	'/usr/lib/x86_64-linux-gnu/libbrotlicommon.so.1.0.7'
]
for peer in peers:
	print(f'{peer} setup start')
	subprocess.run(f'hcp ./Transmission/build/daemon/transmission-daemon {peer}:/', shell=True)
	subprocess.run(f'hcp ./Transmission/build/utils/transmission-remote {peer}:/', shell=True)
	copy_shared_libs(peer, libs)
print('peer setup done')

# TODO seed setup, start torrents
