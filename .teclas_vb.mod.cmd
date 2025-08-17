savedcmd_teclas_vb.mod := printf '%s\n'   teclas_vb.o | awk '!x[$$0]++ { print("./"$$0) }' > teclas_vb.mod
