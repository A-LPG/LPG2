%Headers
/.
        //
        // These functions are only needed in order to use a lexer as
        // a special purpose "diff" program. To do so, one can invoke
        // the main program below directly from the command line.
        //
        private static number LINES = 0,
                           TOKENS = 1;
        private static number differ_mode = LINES; // default
        private static string extension = "";
        private static number changeCount = 0,
                           insertCount = 0,
                           deleteCount = 0,
                           moveCount = 0;

        private static void compareFiles(string old_file, string new_file)
        {
            try
            {
                $action_type old_lexer, new_lexer;
                if (old_file.Equals(""))
                {
                    char [] input_chars = new char[0];
                    old_lexer = new $action_type(input_chars, "null_file");
                }
                else old_lexer = new $action_type(old_file);

                if (new_file.Equals(""))
                {
                    char [] input_chars = new char[0];
                    new_lexer = new $action_type(input_chars, "null_file");
                }
                else new_lexer = new $action_type(new_file);
    
                PrsStream old_stream = new PrsStream(old_lexer.getILexStream());
                old_lexer.lexer(old_stream);
    
                PrsStream new_stream = new PrsStream(new_lexer.getILexStream());
                new_lexer.lexer(new_stream);

                Differ diff = (differ_mode == TOKENS ? (Differ) new DifferTokens(old_stream, new_stream)
                                                     : (Differ) new DifferLines(old_stream, new_stream));
                diff.compare();

                if (diff.getChangeCount() > 0)
                {
                    diff.outputChanges();

                    changeCount += diff.getChangeCount();
                    insertCount += (diff.getInsertCount() + diff.getReplaceInsertCount());
                    deleteCount += (diff.getDeleteCount() + diff.getReplaceDeleteCount());
                    moveCount += diff.getMoveCount();
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
                Console.Error.Write(e.StackTrace);
            }
        }

        private static void compareDirectories(java.io.File old_dir, java.io.File new_dir)
        {
            try
            {
                java.io.File old_file[] = old_dir.listFiles(),
                             new_file[] = new_dir.listFiles();
                java.util.HashMap old_map = new java.util.HashMap();
                for (number i = 0; i < old_file.Length; i++)
                {
                    string name = old_file[i].getName();
                    if (old_file[i].isDirectory() || name.endsWith(extension))
                        old_map.put(name, old_file[i]);
                }

                for (number i = 0; i < new_file.Length; i++)
                {
                    java.io.File file = (java.io.File) old_map.get(new_file[i].getName());
                    if (file != null)
                    {
                        old_map.remove(new_file[i].getName());

                        if (file.isDirectory() && new_file[i].isDirectory())
                             compareDirectories(file, new_file[i]);
                        else compareFiles(file.getPath(), new_file[i].getPath());
                    }
                    else if (new_file[i].isDirectory() ||
                             new_file[i].getName().endsWith(extension))
                    {
                        string s = new_file[i].getName() +
                                   " found in directory " + 
                                   new_dir.getPath() +
                                   " does not exist in directory " +
                                   old_dir.getPath();
                        Console.Error.WriteLine("*Warning: " + s);
                        
                        if (! new_file[i].isDirectory())
                            compareFiles("", new_file[i].getPath());
                    }
                }

                for (java.util.Iterator i = old_map.entrySet().iterator(); i.hasNext(); )
                {
                    java.util.Map.Entry e = (java.util.Map.Entry) i.next();
                    java.io.File file = (java.io.File) e.getValue();
                    
                    string s = file.getName() +
                               " not found in directory " +
                               new_dir.getPath();
                    Console.Error.WriteLine("*Warning: " + s);
                    
                    if (! file.isDirectory())
                        compareFiles(file.getPath(), "");
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
                e.printStackTrace();
            }
        }

        public static void main(string[] args)
        {
            string new_file = null,
                   old_file = null;
            boolean help = false;

            number i;
            for (i = 0; i < args.Length; i++)
            {
                if (args[i].charAt(0) == '-')
                {
                    if (args[i].Equals("-ext"))
                         extension = (i + 1 < args.Length ? args[++i] : "");
                    else if (args[i].Equals("-h"))
                         help = true;
                    else if (args[i].Equals("-l"))
                         differ_mode = LINES;
                    else if (args[i].Equals("-t"))
                         differ_mode = TOKENS;
                }
                else break;
            }
            if (i < args.Length) 
            {
                new_file = args[i++];
                old_file = new_file; // assume only one file is specified
            }
            if (i < args.Length) 
                old_file = args[i++];
            for (; i < args.Length; i++)
                Console.Error.WriteLine("Invalid argument: " + args[i]);

            if (help || (new_file == null &&  old_file == null))
            {
                 Console.Out.WriteLine();
                 Console.Out.WriteLine("Usage: diff [OPTION]... file1 [file2]");
                 Console.Out.WriteLine("Compute stats for file1 or compare file1 to file2 statement by statement.");
                 Console.Out.WriteLine();
                 Console.Out.WriteLine("  -ext s -- if file1 and file2 are directories, compare only files that end\n" +
                                   "            with the extension (suffix) s.");
                 Console.Out.WriteLine("  -h     -- print this help message");
                 Console.Out.WriteLine("  -l     -- compare line by line instead of statement by statement");
                 Console.Out.WriteLine("  -t     -- compare token by token instead of statement by statement");
            }
            else if (old_file.Equals(new_file))
            {
                java.io.File old_dir = new java.io.File(old_file);
                // if (old_dir.isDirectory())
                //     computeStats(old_dir);
                // else computeStats(old_file);

                 Console.Out.WriteLine("*** No difference ***");
                //  Console.Out.WriteLine("    Number of files: " + fileCount);
                //  Console.Out.WriteLine("    Number of lines: " + lineCount);
                //  Console.Out.WriteLine("    Number of types (classes/interfaces): " + (classCount + interfaceCount) + " (" + classCount + "/" + interfaceCount + ")");
                //  Console.Out.WriteLine("    Number of statements: " + statementCount);
                //  Console.Out.WriteLine("    Number of braces (left/right): (" + leftBraceCount + "/" + rightBraceCount + ")");
            }
            else
            {
                java.io.File old_dir = new java.io.File(old_file),
                     new_dir = new java.io.File(new_file);
                if (old_dir.isDirectory() && new_dir.isDirectory())
                     compareDirectories(old_dir, new_dir);
                else compareFiles(old_file, new_file);

                if (changeCount == 0)
                     Console.Out.WriteLine("***** No difference *****");
                else
                {
                     Console.Out.WriteLine("***** " +
                                       changeCount +
                                       " different " +
                                       (changeCount == 1 ? "section" : "sections") + " *****");
                     Console.Out.WriteLine("    " + moveCount    + " statements moved");
                     Console.Out.WriteLine("    " + insertCount  + " statements inserted");
                     Console.Out.WriteLine("    " + deleteCount  + " statements deleted");
                }
            }

            return;
        }
./
%End