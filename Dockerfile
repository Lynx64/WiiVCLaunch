# FROM ghcr.io/wiiu-env/devkitppc:20241128
FROM devkitpro/devkitppc:20260221

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260225 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20260131 /artifacts $DEVKITPRO
# COPY --from=ghcr.io/wiiu-env/libmocha:20240603 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20260208 /artifacts $DEVKITPRO

WORKDIR /project
