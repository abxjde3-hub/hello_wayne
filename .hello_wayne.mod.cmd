savedcmd_hello_wayne.mod := printf '%s\n'   hello_wayne.o | awk '!x[$$0]++ { print("./"$$0) }' > hello_wayne.mod
