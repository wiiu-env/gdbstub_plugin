FROM ghcr.io/wiiu-env/devkitppc:20230218

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20230215 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libkernel:20220904 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libwupsbackend:20230217 /artifacts $DEVKITPRO

WORKDIR project