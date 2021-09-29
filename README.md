# RayTracingEngine
This project aims to provide a simple to use, yet powerful, ray tracing engine. 
The engine features a pipeline structure similar to existing render pipelines.
A user can bind data and shaders to a pipeline, the engine takes care of the
rest.

The project is WIP, however the core functionality can already be tested. Check
the setup section if you want to give it a try.
	
## Milestones
| Milestone                     | Status        |
| ----------------------------- | ------------- |
| Core Engine Structure         | Done!         |
| Accelaration Structures       | Done!         |
| Shader Support                | Done!         |
| Distributed Data Management   | In Progress   |
| Task Based Parallelisation    | In Progress   |
| GPU Support                   | Pending       |
| Distributed Computing         | Pending       |
| Optimisation                  | Pending       |
	
## Linux Setup
This project uses cmake. Inside the project folder run:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```
The last step will install the header files and binaries to your default library 
folders. You can now include the headers in your own project. For more details on
how to get started check the examples.
