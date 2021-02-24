FROM continuumio/miniconda3:latest


ARG BUILD=dev

ENV CONTAINER_USER vignesh
ENV CONTAINER_GROUP camsdk_group
ENV CONTAINER_UID 1000
ENV DEBIAN_FRONTEND=noninteractive
# Add build args
# ARG ARTIFACT_TOKEN
# ENV PIP_EXTRA_INDEX_URL https://mariner-svi:$ARTIFACT_TOKEN@pkgs.dev.azure.com/Mariner-Spyglass/_packaging/mariner-svi/pypi/simple/
# ARG ARG_LOGGING_INSTRUMENTATION_KEY
# ENV LOGGING_INSTRUMENTATION_KEY=$ARG_LOGGING_INSTRUMENTATION_KEY

# Add user to conda
RUN addgroup --gid $CONTAINER_UID $CONTAINER_GROUP && \
    adduser --uid $CONTAINER_UID --gid $CONTAINER_UID $CONTAINER_USER --disabled-password  && \
    mkdir -p /opt/conda && chown $CONTAINER_USER /opt/conda

RUN apt-get update && apt-get install -y software-properties-common rsync
RUN apt-get update && apt-get install -y git libglib2.0-dev graphviz nano && apt-get update
# RUN pip install svi_core_2

RUN /bin/bash -c "if [[ $BUILD == 'dev' ]] ; then echo \"Dev Build\" && pip install graphviz watchdog[watchmedo] nbdev nbconvert==5.* aquirdturtle_collapsible_headings matplotlib opencv-python; fi "

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
ENV PATH=${PATH}:/home/$CONTAINER_USER/python_sdk/lib/Linux64_x64

# COPY --chown=$CONTAINER_USER:$CONTAINER_GROUP misc/themes.jupyterlab-settings /home/$CONTAINER_USER/.jupyter/lab/user-settings/@jupyterlab/apputils-extension/
# COPY --chown=$CONTAINER_USER:$CONTAINER_GROUP misc/shortcuts.jupyterlab-settings /home/$CONTAINER_USER/.jupyter/lab/user-settings/@jupyterlab/shortcuts-extension/
# COPY --chown=$CONTAINER_USER:$CONTAINER_GROUP misc/tracker.jupyterlab-settings /home/$CONTAINER_USER/.jupyter/lab/user-settings/@jupyterlab/notebook-extension/

USER $CONTAINER_USER
ENV PATH="/home/$CONTAINER_USER/.local/bin:${PATH}"
COPY --chown=$CONTAINER_USER:$CONTAINER_GROUP . /home/$CONTAINER_USER/camsdk

# RUN /bin/bash -c "if [[ $BUILD == 'prod' ]] ; then echo \"Production Build\" && pip install svi_core_2 --upgrade  && pip install .; fi"
# RUN /bin/bash -c "if [[ $BUILD == 'dev' ]] ; then echo \"Development Build\" && pip install svi_core_2 --upgrade && pip install -e \".[dev]\"; fi"

RUN echo '#!/bin/bash\njupyter lab --ip=0.0.0.0 --port=9501 --allow-root --no-browser  --NotebookApp.token='' --NotebookApp.password=''' >> ../run_jupyter.sh

USER $CONTAINER_USER
RUN /bin/bash -c "pip install -e ."

RUN chmod u+x ../run_jupyter.sh
# RUN chmod u+x run.sh

#ENTRYPOINT "./run.sh"
#CMD ["/bin/bash","-c"]