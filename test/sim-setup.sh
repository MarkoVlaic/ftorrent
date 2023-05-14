#! /bin/sh

# Before doing anything check for root rights.
if [ `id -u` -ne  0 ]; then
    echo "You must be root to run this script."
    exit 1
fi

# change to top level project dir 
cd ../../

# setup tracker
hcp opentracker/opentracker.debug tracker:/
#himage tracker ./opentracker.debug &
echo "tracker setup done"

# setup client
hcp /usr/lib/* client:/usr/lib
hcp /usr/local/lib/* client:/usr/local/lib
hcp ftorrent/build/demo/demo client:/ftorrent
hcp ./ftorrent/test/test-file.torrent client:/
echo "client setup done"
