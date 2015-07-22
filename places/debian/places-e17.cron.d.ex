#
# Regular cron jobs for the places-e17 package
#
0 4	* * *	root	[ -x /usr/bin/places-e17_maintenance ] && /usr/bin/places-e17_maintenance
