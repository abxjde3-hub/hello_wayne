savedcmd_wayne_monitor.mod := printf '%s\n'   wayne_monitor.o | awk '!x[$$0]++ { print("./"$$0) }' > wayne_monitor.mod
