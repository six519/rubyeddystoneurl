rubyeddystoneurl
================

Ruby C Extension To Scan Eddystone-URL Bluetooth Low Energy Beacons

.. image:: http://ferdinandsilva.com/static/rubyeddystoneurl.jpg

Install Requirements For Linux (Ubuntu, Debian)
===============================================
::

	sudo apt-get install libbluetooth-dev

Install Requirements For Linux (Red Hat, CentOS, Fedora)
========================================================
::

	sudo yum install bluez-libs-devel

Installing Through RubyGems
===========================
::

	sudo gem install rubyeddystoneurl

Ruby Sample Usage
=================
::

	require 'rubyeddystoneurl'
	devices = discover 10