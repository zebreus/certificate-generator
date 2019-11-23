# certificate-generator
Generates certificates from templates.

## Status

[<p align="center"><img src="https://images.madmanfred.com/certificate-generator-status.jpg"></p>](https://concourse.einhorn.jetzt/teams/main/pipelines/certificate-generator)

## Building the project

### Server
#### Docker
The easiest way to get the server is to run `make docker` to build a docker container containing the server. After building you can start the server with `docker run -p 9090:9090 certgen`.
#### Executable
The server executable depends on thrift, boost and a local installation of texlive.
To build the server executable run `make thrift` and `make server`. The executable will be build as `out/server`.

### Client
The server executable depends on thrift and boost.
To build the server executable run `make thrift` and `make client`. The executable will be build as `out/client`.

### Local installation
The local executable depends on a local installation of texlive.
To build the executable run `make local`. The executable will be build as `out/local`.


## Writing configuration files
The batch configuration files are json files.
The base object contains an array of students, an array of templates, an array of resources and strings for global variables and some properties for configuration.

### Students
Here you specify the values for substitution.
Each object in this array will be used with every template file to generate a pdf document.
An array of objects, containing properties for each string that should be replaced and an array of objects for each optional or table.
#### Examples:
Two students

    "students":[
    	{
    		"name":"max",
    		"age":"30"
    	},
    	{
    		"name":"simon",
    		"age":"17"
    	}
    ]
A student with a optional containing two entries

    "students":[
    	{
    		"name":"max",
    		"cats":[
    			{
    				"name":"cat1",
    				"age":"3"
    			},
    			{
    				"name":"cat2",
    				"age":"5"
    			}
    		]
    	}
    ]

### Templates
Here you specify the template files.
An array of strings
#### Example:

    "templates":[
    	"./template1.tex",
    	"/path/to/template2.tex"
    ]

### Resources
Here you specify which resources are needed to compile the template files.
They will be copied into the directory where latex is executed
An array of strings
#### Example:

    "resources":[
    	"./path/font.ttf",
    	"/path/to/image.png"
    ]

### Global variables:
Global variables are used for substitutions that are independent from the students.
All properties of the base object, that are not used for configuration are global variables
#### Example:

    "date":"1.1.2019",
    "template_author":"Simon"

### Configuration variables:
outputDirectory: string, Specifies, where the pdfs should be put
workingDirectory: string, This directory will be used to put some files.

#### Examples

    "outputDirectory":"./output",
    "workingDirectory":"./working"

These properties are required:
students, templates, outputDirectory

See demobatch.json for a complete example


## Writing template files
A template file is just a normal .tex file with the special commands listed below. Those commands will be replaced with the appropriate values by the certificate-generator. To create templates you should include the certificate-generator package, because it adds placeholders for the commands, so you can compile your tex file.

### Commands:
#### substitution: 
\substitude gets replace by a value with the same name from the configuration file. You can define where to look for the value, by setting the first option to auto, global, student or the name of an optional. If you do not set it, or set it to auto, the value will be searched everywhere. If multiple entrys of the same name exist, auto will first search in the optional it is currently in, then the student entries and last the global entries.

    \substitude{name_of_value}
    \substitude[auto|student|global|name_of_optional]{name_of_value}
    
#### optional:
\optional defines a section that will be inserted 0 or more times, depending on the number of entries in the array with the optional name in the current student.
 `\optional{name_of_optional}{content}`

### Examples:
For the following examples this configuration will be used:

    {
    "students":[
    	{
    	"name":"Max",
    	"surname":"Mustermann",
    	"date":"14. August 1998",
    	"tasks":[
    		{
    		"name":"showing up",
    		"grade":"2.3"
    		},
    		{
    		"name":"breathing",
    		"grade":"2.7"
    		}
    	]
    	}
    ],
    "name":"Manfred"
    }

#### Example 1:
Input:

    \substitude{name} \substitude[student]{surname} \substitude[global]{name}

Output:

    Max Mustermann Manfred

#### Example 2:
Input:

    \optional{tasks}{ \substitude[student]{name} : \substitude{name} \substitude{grade}
    }
Output:

    Max : showing up 2.3
    Max : breathing 2.7

#### Example 3:
Input:

    \substitude{name} \substitude{surname} on \substitude{date} you have completed these tasks:
    \begin{tabular}{ l | c }
    \hline
    Task & Rating \\ \hline
    \optional{tasks}{
    \substitude{name} & \substitude{grade} \\ \hline
    }
    \end{tabular}
    Certified by \substitude[global]{name}

Output:

    Max Mustermann on 14. August 1998 you have completed these tasks:
    \begin{tabular}{ l | c }
    \hline
    Task & Rating \\ \hline
    showing up & 2.3 \\ \hline
    breathing & 2.7 \\ \hline
    \end{tabular}
    Certified by Manfred

### What you can't do:
 - Naming optionals "local" "global" or "optional"
 - Have entrynames containint the symbols '"' '<' '>' and '/'
 - Use wrong syntax
 - Rely on this list to be complete





