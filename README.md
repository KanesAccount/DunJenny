# DunJenny
This project implements procedural generation through the use of transformational graph grammars to develop a level generation engine for the creation of rouge-like game levels via rules specified by the designer. The engine has been developed using the Visual Studio IDE and written in C++. 

The main challenges that this project presented were in the development of the hill climbing algorithm application within the graph derivation stage, due to the use of random node identifiers a large portion of time was spent tracing the steps of the algorithm on paper to identify why the solution was not producing the expected results.
With the use of the IMGUI translating the rule, node and edge from the back-end graph grammar system was a relatively seamless transition from logic to interface. 
While the solution presented in this project does not generate entirely populated dungeon levels, it qualifies as a base structure for the generation of abstract game levels on which additional objectives can be applied. 

The produced product is capable of creating a range of spaces that are suitable for use in creating a dungeon level, or for the conversion and production of a written mission utilising the generalised purpose of the system, and provides a method of doing so through the designer-assisted lock & key system. 
Through tweaking the supplied variables a user can generate a range of graph styles which can then be further adapted or expanded based on the required needs or theme. 
Overall the outcome of the project has created a useful tool in the design and creation of game levels, with a good basis for future expansion.

Had the integration of locks and keys been enveloped into the back end of the graph grammar systems algorithm design stage then the final implementation would align more with the initial goals of the project.
