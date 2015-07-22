#
# Regular cron jobs for the engage package
#
0 4	* * *	root	[ -x /usr/bin/engage_maintenance ] && /usr/bin/engage_maintenance
