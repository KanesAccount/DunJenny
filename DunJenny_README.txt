DunJenny \README/


The DunJenny project makes use of Cmake to create the Visual Studio Solution.

Note: C++ 17 and Windows 10 SKD is required to run the application!

To create the VSproject, open the Cmake GUI, select the DunJenny folder as
the source folder, and select the 'Build' folder as the target.

Press configure and ensure there are no errors, then generate to create the solution.

Once the Build folder is populated, open the solution and run ALL_BUILD

This should compile the project and create the DunJenny.exe & DunJenny_d.exe

If the cMake build should fail, or the release version should crash, 
utilise the debug version in:
'FinalBuild/bin/DungGenerator_d.exe'


INSTRUCTIONS:

To generate a new map, first select file->Generate new map
This will populate the Available rules section with three test rules.
The breakdown of how these rules are constructed can be seen by clicking the rule name
under 'Available Rules'

Once the rules have been added right-click anywhere in the graph map section and
select 'Room'. This adds the initial node the algorithm will generate a graph from.

Now select tools->regenerate map

A new graph will be generated, to focus the view zoom out, click & drag to select
all nodes in the graph, once selected press 'f' to pull focus and press 'f' again
to zoom in.

The 'z' key can now be pressed to view the flow of the map.

To inspect the maps data, select tools->Open Data Viewer

The shortcut key (x + enter) will regenerate a map if there is at least one node 
in the graph.

Tweak the generation variables to achieve different graph outputs.

At any point the log can be found by tools->Show Log
This details the construction of the graph with 'Show Graph'
and the strategy applied with 'Show Generation Strategy'

'Show Rules Added' is supplied with dummy text from json serializing/deserializing.

Open the rule builder to create a rule. Read the instructions before starting!

Thanks and have fun :)