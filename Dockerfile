FROM ghcr.io/wiiu-env/devkitppc:20250608

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20250608 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20240426 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmocha:20250608 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20241012 /artifacts $DEVKITPRO

WORKDIR /project
