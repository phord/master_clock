#!/bin/sh

set -ex

# Install and activate the master clock service on a Debian raspi
chmod a+x clock.py clock-service clock-status.py install.sh setclock.py SimplexProtocol.py

sudo apt-get install python3-pip
sudo pip3 install ntplib
sudo cp master-clock.service /etc/systemd/system/
sudo chmod 0644 /etc/systemd/system/master-clock.service
sudo systemctl daemon-reload
sudo systemctl enable master-clock.service

# Start the service
sudo systemctl start master-clock.service

echo "You should reboot to confirm the service starts up normally"
