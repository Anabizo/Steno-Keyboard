savedcmd_steno.mod := printf '%s\n'   steno.o | awk '!x[$$0]++ { print("./"$$0) }' > steno.mod
