%Headers
/.
        //
        // These functions are only needed in order to use a lexer as
        // a special purpose "diff" program. To do so, one can invoke
        // the main program below directly from the command line.
        //
        inline static int LINES = 0,
                           TOKENS = 1;
         inline  static int differ_mode = LINES; // default
        inline   static std::wstring extension = L"";
        inline   static int changeCount = 0,
                           insertCount = 0,
                           deleteCount = 0,
                           moveCount = 0;

         static void compareFiles(const std::wstring& old_file,const std::wstring& new_file)
        {
            try
            {
                $action_type* old_lexer;
                 $action_type * new_lexer;
                if (old_file==(L""))
                {
                    shared_ptr_wstring input_chars;
                    old_lexer = new $action_type(input_chars, L"null_file");
                }
                else old_lexer = new $action_type(old_file);

                if (new_file==(L""))
                {
                     shared_ptr_wstring input_chars;
                    new_lexer = new $action_type(input_chars, L"null_file");
                }
                else new_lexer = new $action_type(new_file);
    
                PrsStream* old_stream = new PrsStream(old_lexer->getLexStream());
                old_lexer->lexer(old_stream);
    
                PrsStream new_stream = new PrsStream(new_lexer->getLexStream());
                new_lexer->lexer(new_stream);

                Differ* diff = (differ_mode == TOKENS ?  new DifferTokens(old_stream, new_stream)
                                                     :new DifferLines(old_stream, new_stream));
                diff->compare();

                if (diff->getChangeCount() > 0)
                {
                    diff->outputChanges();

                    changeCount += diff->getChangeCount();
                    insertCount += (diff->getInsertCount() + diff->getReplaceInsertCount());
                    deleteCount += (diff->getDeleteCount() + diff->getReplaceDeleteCount());
                    moveCount += diff->getMoveCount();
                }
            }
            catch (std::exception& e)
            {
                std::cout << (e.what());
                
            }
        }

         static void compareDirectories(java.io.File old_dir, java.io.File new_dir)
        {
            try
            {
                java.io.File old_file[] = old_dir.listFiles(),
                             new_file[] = new_dir.listFiles();
                java.util.HashMap old_map = new java.util.HashMap();
                for (int i = 0; i < old_file.length; i++)
                {
                    String name = old_file[i].getName();
                    if (old_file[i].isDirectory() || name.endsWith(extension))
                        old_map.put(name, old_file[i]);
                }

                for (int i = 0; i < new_file.length; i++)
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
                        String s = new_file[i].getName() +
                                   " found in directory " + 
                                   new_dir.getPath() +
                                   " does not exist in directory " +
                                   old_dir.getPath();
                        System.err.println("*Warning: " + s);
                        
                        if (! new_file[i].isDirectory())
                            compareFiles("", new_file[i].getPath());
                    }
                }

                for (java.util.Iterator i = old_map.entrySet().iterator(); i.hasNext(); )
                {
                    java.util.Map.Entry e = (java.util.Map.Entry) i.next();
                    java.io.File file = (java.io.File) e.getValue();
                    
                    String s = file.getName() +
                               " not found in directory " +
                               new_dir.getPath();
                    System.err.println("*Warning: " + s);
                    
                    if (! file.isDirectory())
                        compareFiles(file.getPath(), "");
                }
            }
            catch (Exception e)
            {
                System.err.println(e.getMessage());
                e.printStackTrace();
            }
        }

./
%End