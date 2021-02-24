FROM ubuntu:18.04

ARG BUILD=dev

# Configure environment
ENV CONDA_DIR /opt/conda
ENV PATH $CONDA_DIR/bin:$PATH
ENV SHELL /bin/bash
ENV CONTAINER_USER drtools
ENV CONTAINER_UID 1000
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

# Configure User
ENV CONTAINER_USER vignesh
ENV CONTAINER_GROUP camsdk_group
ENV CONTAINER_UID 1000
ENV DEBIAN_FRONTEND=noninteractive

# Configure Miniconda
ENV MINICONDA_VER 4.5.4
ENV MINICONDA Miniconda3-$MINICONDA_VER-Linux-x86_64.sh
ENV MINICONDA_URL https://repo.continuum.io/miniconda/$MINICONDA
ENV MINICONDA_MD5_SUM 0c28787e3126238df24c5d4858bd0744

RUN addgroup --gid $CONTAINER_UID $CONTAINER_GROUP && \
    adduser --uid $CONTAINER_UID --gid $CONTAINER_UID $CONTAINER_USER --disabled-password  && \
    mkdir -p /opt/conda && chown $CONTAINER_USER /opt/conda

RUN apt-get update && apt-get install -y software-properties-common rsync
RUN apt-get update && apt-get install -y git libglib2.0-dev graphviz nano curl libpython3.6-dev

# Install conda
RUN cd /tmp && \
    mkdir -p $CONDA_DIR && \
    curl -L $MINICONDA_URL  -o miniconda.sh && \
#    echo "$MINICONDA_MD5_SUM  miniconda.sh" | md5sum -c - && \
    /bin/bash miniconda.sh -f -b -p $CONDA_DIR && \
    rm miniconda.sh && \
    $CONDA_DIR/bin/conda install --yes conda==$MINICONDA_VER

RUN conda upgrade -y pip && \
    conda config --add channels conda-forge && \
    conda clean --all

RUN /bin/bash -c "if [[ $BUILD == 'dev' ]] ; then echo \"Dev Build\" && pip install graphviz watchdog[watchmedo] nbdev nbconvert==5.* aquirdturtle_collapsible_headings matplotlib; fi "

WORKDIR /home/$CONTAINER_USER/camsdk
# Josiah: This block will take a while. If you want, you can comment it out and see if you can install everything  maybe
# using pip install --user ?
RUN chown $CONTAINER_USER:$CONTAINER_GROUP -R /opt/conda/bin
RUN chown $CONTAINER_USER:$CONTAINER_GROUP -R /opt/conda/lib/python3.*
# This should be instant
RUN chown $CONTAINER_USER:$CONTAINER_GROUP -R /home/$CONTAINER_USER
RUN chown $CONTAINER_USER:$CONTAINER_GROUP -R /home/$CONTAINER_USER/camsdk
# Same long processing issue here...
RUN chmod 755 -R /opt/conda/bin
RUN chmod 755 -R /opt/conda/lib/python3.*
# This should be instant
RUN chmod 755 -R /home/$CONTAINER_USER
RUN chmod 755 -R /home/$CONTAINER_USER/camsdk

ADD python_sdk_18.04 /home/$CONTAINER_USER/python_sdk
ENV PYTHONPATH=${PYTHONPATH}:/home/$CONTAINER_USER/python_sdk/lib/Linux64_x64

USER $CONTAINER_USER
ENV PATH="/home/$CONTAINER_USER/.local/bin:${PATH}"
COPY --chown=$CONTAINER_USER:$CONTAINER_GROUP . /home/$CONTAINER_USER/camsdk

RUN echo '#!/bin/bash\njupyter lab --ip=0.0.0.0 --port=9501 --allow-root --no-browser  --NotebookApp.token='' --NotebookApp.password=''' >> ../run_jupyter.sh

RUN /bin/bash -c "pip install -e ."

RUN chmod u+x ../run_jupyter.sh