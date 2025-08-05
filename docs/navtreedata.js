/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Mantis App", "index.html", [
    [ "Getting Started", "index.html", null ],
    [ "Setting Up Mantis", "setup.html", null ],
    [ "Command Line Usage", "cli.html", [
      [ "🧭 mantisapp CLI Reference", "cli.html#autotoc_md8", [
        [ "⚙️ Global Options", "cli.html#autotoc_md10", null ],
        [ "🚀 serve Command", "cli.html#autotoc_md12", null ],
        [ "👤 admins Command", "cli.html#autotoc_md14", null ],
        [ "🛠️ migrate Command *(WIP)*", "cli.html#autotoc_md16", null ],
        [ "</blockquote>", "cli.html#autotoc_md17", null ],
        [ "🔄 sync Command *(Reserved)*", "cli.html#autotoc_md18", null ],
        [ "📝 Notes", "cli.html#autotoc_md20", null ],
        [ "📚 See Also", "cli.html#autotoc_md22", null ]
      ] ]
    ] ],
    [ "REST API Reference Guide", "rest_api.html", null ],
    [ "API Access Rules", "rest_api_rules.html", null ],
    [ "Architecture", "architecture.html", [
      [ "Mantis Architecture Overview", "architecture.html#autotoc_md74", [
        [ "📐 Core Principles", "architecture.html#autotoc_md76", null ],
        [ "🧱 High-Level Architecture", "architecture.html#autotoc_md78", null ],
        [ "🧩 Components", "architecture.html#autotoc_md80", [
          [ "1. <strong>Database Layer</strong>", "architecture.html#autotoc_md81", null ],
          [ "2. <strong>API Router</strong>", "architecture.html#autotoc_md82", null ],
          [ "3. <strong>Authentication Module</strong>", "architecture.html#autotoc_md83", null ],
          [ "4. <strong>Middleware System</strong>", "architecture.html#autotoc_md84", null ],
          [ "5. <strong>Sync Engine</strong>", "architecture.html#autotoc_md85", null ],
          [ "6. <strong>Static File Server</strong>", "architecture.html#autotoc_md86", null ],
          [ "7. <strong>System Tables</strong>", "architecture.html#autotoc_md87", null ]
        ] ],
        [ "🔄 Sync Model", "architecture.html#autotoc_md89", null ],
        [ "⚙️ Deployment Modes", "architecture.html#autotoc_md91", [
          [ "➕ Embeddable Library", "architecture.html#autotoc_md92", null ],
          [ "🚀 Standalone Binary", "architecture.html#autotoc_md93", null ]
        ] ],
        [ "📁 File Structure Overview", "architecture.html#autotoc_md95", null ],
        [ "📌 Extensibility Ideas", "architecture.html#autotoc_md97", null ],
        [ "🏁 Conclusion", "architecture.html#autotoc_md99", null ]
      ] ]
    ] ],
    [ "Embedding Mantis", "embedding.html", null ],
    [ "Running in Docker", "docker.html", [
      [ "Dockerizing MantisApp", "docker.html#autotoc_md117", null ],
      [ "Limitations", "docker.html#autotoc_md118", null ],
      [ "Dockerfile", "docker.html#autotoc_md119", null ],
      [ "Docker Compose", "docker.html#autotoc_md120", null ]
    ] ],
    [ "Engine Sync", "sync.html", [
      [ "Mantis Sync Engine Design (NOT IMPLEMENTED YET!)", "sync.html#autotoc_md121", [
        [ "🔄 Sync Model Overview", "sync.html#autotoc_md123", null ],
        [ "⚙️ Sync Workflow", "sync.html#autotoc_md125", [
          [ "1. <strong>Change Detection</strong>", "sync.html#autotoc_md126", null ],
          [ "2. <strong>Conflict Resolution</strong>", "sync.html#autotoc_md127", null ],
          [ "3. <strong>Transport Layer</strong>", "sync.html#autotoc_md128", null ],
          [ "4. <strong>Sync Tables</strong>", "sync.html#autotoc_md129", null ]
        ] ],
        [ "🔐 Security", "sync.html#autotoc_md131", null ],
        [ "🧪 Sync Testing", "sync.html#autotoc_md133", null ],
        [ "🔧 Configuration Example", "sync.html#autotoc_md135", null ],
        [ "📌 Sync Customization", "sync.html#autotoc_md137", null ],
        [ "🏁 Summary", "sync.html#autotoc_md139", null ]
      ] ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classmantis_1_1SysTablesUnit.html#a1f220eab6789e7793b9887298837a447",
"index.html",
"structmantis_1_1ViewTable.html#abb14ce2181837e55c28327a3cff9b0ce"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';