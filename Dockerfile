FROM wiiuenv/devkitppc:20230218

COPY --from=wiiuenv/wiiupluginsystem:20230215 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libkernel:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20230217 /artifacts $DEVKITPRO

WORKDIR project