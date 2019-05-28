# certificate-generator
Generates certificates from templates

Usage:
-t template.tex : Specify template file
-b batchfile.json : Specify batch config file
-c configfile.json : Specify config file
-o outputdirectory : Specify output directory
-w workingdirectory : Specify working directory
-v : verbose
-h : help


dependencies:
nlohmann-json

substitution syntax:
globaler wert: <sub name="NAME_DES_GLOBALEN_WERTS" type="global"/>
lokaler wert: <sub name="NAME_DES_LOKALEN_WERTS" type="local" />
tabelle:	<table name="NAME_DER_TABELLE">Hier können auch tabellenwerte verwendet werden.</table>
tabellenwert: <sub name="NAME_DES_TABELLENWERTS" type="TABELLENNAME" />

ZU BESPRECHEN:
-syntax entscheiden
-struktur besprechen

TODO:
-globale eigenschaften konzeptionieren
-globale eigenschaften implementieren
-namen bestimmen
-parser fertig implementieren
-pdflatex ausführung verbessern
-template schreiben
-code strukturieren
-erweitern
-schriftart

HOW TO WRITE BATCHCONFIGURATION FILES
The batch configuration files are json files.
The base object contains an array of students, an array of templates, an array of resources and strings for global variables and some properties for configuration

students:
Here you specify the values for substitution.
Each object in this array will be used with every template file to generate a pdf document.
An array of objects, containing properties for each string that should be replaced and an array of objects for each table
example: Two students
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
example: a student with a table containing two entries
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

templates:
Here you specify the template files.
An array of strings
example:
"templates":[
	"./template1.tex",
	"/path/to/template2.tex"
]

templates:
Here you specify which resources are needed to compile the template files.
They will be copied into the directory where latex is executed
An array of strings
example:
"resources":[
	"./path/font.ttf",
	"/path/to/image.png"
]

global variables:
Global variables are used for substitutions that are independent from the students.
All properties of the base object, that are not used for configuration are global variables
example:
"date":"7.12.2019"

configuration variables:
outputDirectory: string, Specifies, where the pdfs should be put
workingDirectory: string, This directory will be used to put some files. //TODO If not specified one will be created on /tmp
verbose: true or false, if true more output produced

These properties are required:
students, templates, outputDirectory

See demobatch.json for a complete example


HOW TO WRITE TEMPLATE FILES
short version:
substitution: <sub name="NAME_OF_VALUE" type="global|local||NAME_OF_A_TABLE"/>
table:	<table name="NAME_DER_TABELLE">CONTENT</table>

long version:
To use substitutions in your tex files insert a sub tag.
The name property in the sub tag specifies the name of the field the value will be taken from
The type property specifies where the value will be taken from
Options for type are:
global : the value will be taken from the first level of your configuration.
local : the value will be taken from the student that is used for the certificate
If you don't specify a type, local will be assumed
NAME_OF_A_TABLE : the value will be taken from the table with the given name. Only valid inside a matching table tag

To generate arrays use the table tag as seen in Example 3.


Examples:
For the following examples this student will be used:
{
"name":"Max",
"surname":"Mustermann",
"date":"8.4.2019",
"achievements":[
	{
	"name":"example",
	"grade":"A"
	},
	{
	"name":"another example",
	"grade":"D"
	}
]
}

Example 1:
Input:
<sub name="surname" type="local" /> <sub name="name" type="local" />
Output:
Mustermann Max

Example 2:
Input:
<sub name="name"/> <sub name="surname"/> graduated on <sub name="date" />
Output:
Max Mustermann graduated on 8.4.2019

Example 3:
Input:
<table name="achievements"><sub name="name"/> achieved grade <sub name="grade" type="achievements"/> in <sub name="name" type="achievements"/> | </table>
Output:
Max achieved grade A in example | Max achieved grade D in another example |

What is not allowed:
tables must not be named "local" "global" or "table"
no property is allowed to contain the symbols '"' '<' '>' and '/'
you must not use wrong syntax, lol
