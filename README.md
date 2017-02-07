nvidia-docker build .

nvidia-docker run -v /home/brett/Documents/source/opencl-docker-info/src/images:/opt/opencl/src/images ff5ac49351d6

Installed Nvidia drivers, after much messing around I figured out the best way was to go into Software & Updates => Additional drivers and there was the Nvidia card using the Nouveau open source drivers (which cause all sorts of issues with some later steps). There you can choose to use the proprietary Nvidia drivers. I spent hours trying to do this from the command line before figuring this out, ugghhh. The main problem is that you need to disable the Nouveau drivers before installing anything else, this is non-trivial from the command line.

Nvidia actually provide their own Docker engine for running containers that need access to the GPU. I haven’t looked into exactly how specialised this is but it does rely on docker-engine as a dependency, so I guess it just augments it. Speaking of, I needed to install docker-engine from the third party docker repo (Provided by Docker) rather than apt-get install docker-io which uses the docker engine that ships with Ubuntu. Nvidia-docker doesn’t recognize the ‘io’ engine, it’s also not the latest.
So, installed the correct docker-engine and Nvidia drivers, then the nvidia-docker deb file from the nvida-docker github install instructions failed because it doesn’t support 16.10 yet. I was able to grab the deb spec from github and modify the ‘From’ line at the top from 14.04 to 16.04. Then I installed make and was able to build the deb file with ‘make deb’. Then I could install it with: sudo dpkg -i <name of deb file created from ‘make deb’>
This successfully installed nvidia-docker from this modified deb file.
