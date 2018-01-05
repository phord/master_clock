#!/bin/sh

# Install and activate the master clock service on a Debian raspi

sudo cp master-clock.service /lib/systemd/system/
sudo chmod 0644 /lib/systemd/system/master-clock.service
sudo systemctl daemon-reload
sudo systemctl enable master-clock.service

echo "Now reboot to confirm the service starts up normally"

# Start the service
./clock-service &
