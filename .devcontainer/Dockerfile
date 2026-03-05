FROM ghcr.io/anacierdem/n64-unfloader:latest AS unf

# Need to pin this as we don't yet have arm builds on GH actions
FROM ghcr.io/dragonminded/libdragon@sha256:54b5b46c866eef3c5a22e1996a1773fd718c96c50fe3209a8cb8fdc91a1565ed AS libdragon

FROM ghcr.io/anacierdem/ares:latest AS ares

RUN apt-get update && apt-get install \
    build-essential \
    git \
    # UNF dependencies
    libftdi1-dev \
    -yq

# Setup paths for the libdragon toolchain
ENV N64_INST=/home/ubuntu/.local

COPY --from=unf --chown=ubuntu /unf/UNFLoader/UNFLoader ${N64_INST}/bin/UNFLoader
COPY --from=libdragon --chown=ubuntu /n64_toolchain ${N64_INST}
RUN cp -r /ares/build/rundir/. ${N64_INST}

USER ubuntu