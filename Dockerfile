FROM devkitpro/devkitppc:20260221

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260418 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20260404 /artifacts $DEVKITPRO
# COPY --from=ghcr.io/wiiu-env/libmocha:20240603 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20260331 /artifacts $DEVKITPRO

WORKDIR /project
