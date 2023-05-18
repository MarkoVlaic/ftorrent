import subprocess
import sys

def run_cmd(cmd):
	subprocess.run(cmd, shell=True)

torrent_file_name = 'test-file.torrent'
if len(sys.argv) > 1:
	torrent_file_name = sys.argv[1]

num_peers = 6
peers = [f'peer{i + 1}' for i in range(num_peers)]
for peer in peers:
	run_cmd(f'himage {peer} transmission-remote 0.0.0.0:9091 --add ./{torrent_file_name}')

run_cmd(f'himage client ./ftorrent ./{torrent_file_name}')
