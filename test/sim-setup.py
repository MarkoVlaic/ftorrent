import os
import subprocess
import sys

def run_cmd(cmd):
	subprocess.run(cmd, shell=True)

def copy_shared_libs(node, paths):
	for path in paths:
		print(f'copy {path}')
		to_path = path
		
		if to_path[-1] == '*':
			to_path = to_path[:-1]
		
		run_cmd(f'hcp {path} {node}:{to_path}')
	
	lib_dirs = set()
	for path in paths:
		i = path.rfind('/')
		lib_dirs.add(path[:i+1])
	
	for d in lib_dirs:
		run_cmd(f'himage {node} ldconfig -n {d}')
	
# go to top level project dir
os.chdir('../..')

# setup tracker
print('tracker setup start')
run_cmd('hcp opentracker/opentracker.debug tracker:/opentracker')
run_cmd('himage tracker ./opentracker &')
print('tracker setup done')

torrent_file_name = 'test-file.torrent'
test_file_name = 'test-file.png'
if len(sys.argv) > 1:
	torrent_file_name = sys.argv[1]
	test_file_name = sys.argv[2]
torrent_file_path = f'./ftorrent/test/{torrent_file_name}'
test_file_path = f'./ftorrent/test/{test_file_name}'

# setup client
print('client setup start')
run_cmd('hcp ./ftorrent/build/demo/demo client:/ftorrent')
libs = [
	#'/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.28',
	'/usr/lib/liburing.so.2.4',
	'/usr/lib/liburing-ffi.so.2.4',
	'/usr/lib/liburing.a',
	'/usr/lib/liburing-ffi.a',
	'/usr/local/lib/*',
	#'/usr/lib/x86_64-linux-gnu/*'
]
copy_shared_libs('client', libs)
run_cmd(f'hcp {torrent_file_path} client:/')
run_cmd('himage client sh -c "ulimit -l 65535"')
print('client setup done')

# setup peers
num_peers = 6
peers = [f'peer{i + 1}' for i in range(num_peers)]
for peer in peers:
	print(f'{peer} setup start')
	run_cmd(f'hcp {torrent_file_path} {peer}:/{torrent_file_name}')
	run_cmd(f'hcp ./ftorrent/test/transmission-settings.json {peer}:/var/lib/transmission-daemon/info/settings.json')
	run_cmd(f'himage {peer} service transmission-daemon start')
print('peer setup done')

print('seed setup start')
seed = peers[0]
run_cmd(f'hcp {test_file_path} {seed}:/var/lib/transmission-daemon/downloads')
run_cmd(f'himage {seed} transmission-remote 0.0.0.0:9091 -t 1 -v')
print('seed setup done')
