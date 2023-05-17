import subprocess

def run_cmd(cmd):
	subprocess.run(cmd, shell=True)

num_peers = 6
peers = [f'peer{i + 1}' for i in range(num_peers)]
for peer in peers:
	run_cmd(f'himage {peer} transmission-remote 0.0.0.0:9091 --add ./test-file.torrent')

run_cmd('himage client ./ftorrent')
