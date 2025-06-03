# APRILContent
Algorithm of Particle Reconstruction for ILC - implementation with PandoraSDK

How to compile

1) source standard init_ilcsoft.sh script
2) Make sure that you have mlpack, boost and armadillo installed.
boost comes with ilcsoft. to install both mlpack and armadillo, do (example with mlpack 4.3.0 )
    - a) wget https://mlpack.org/files/mlpack-4.3.0.tar.gz
    - b) tar xzvf mlpack-4.3.0.tar.gz
    - c) cd mlpack-4.3.0/
    - d) mkdir build; cd build
    - e) cmake -DDOWNLOAD_DEPENDENCIES=ON -DCMAKE_INSTALL_PREFIX=../install ..
    - f) make install
3) Clone the repository
4) cd APRILContent; mkdir build; cd build
5) PKG_CONFIG_PATH=/path/to/MLPACK/.../lib64/pkgconfig cmake -C ${ILCSOFT}/ILCSoft.cmake -DPANDORA_MONITORING=ON .. -DBoost_INCLUDE_DIR=/path/to/boost/include -DARMADILLO_INCLUDE_DIR=/path/to/mlpack/build/deps/armadillo-10.3.0/include
6) make install

