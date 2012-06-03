using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace run_if_newer
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 3)
            {
                System.Console.WriteLine("arguments required: fileA fileB command optional command params");
                System.Console.WriteLine("example: \na.cu a.cu.ptx nvcc a.cu --ptx --include-path %CUDA_INC_PATH% --output-file a.cu.ptx");
                System.Environment.Exit(1);
            }
            string fileInp = args[0];
            string fileOut = args[1];
            string command = args[2];

            string command_args = "";
            if (args.Length >= 3)
            {
                for (int i = 3; i < args.Length; i++)
                {
                    command_args += args[i] + " ";
                }
            }
            

            if (System.IO.File.Exists(fileInp) == false)
            {
                System.Console.WriteLine("input file does not exist! ");
                System.Environment.Exit(1);
            }
            if (System.IO.File.Exists(fileOut) == false)
            {
                System.Console.WriteLine("out file does not exist, running command...");
                runCommand(command, command_args);
            }
            else
            {
                DateTime inpTime = System.IO.File.GetLastWriteTime(fileInp);
                DateTime outTime = System.IO.File.GetLastWriteTime(fileOut);
                if (inpTime > outTime)
                {
                    runCommand(command, command_args);
                }
                else
                {
                    System.Console.WriteLine("not running command, out file newer than inp");
                }
            }


            //System.Console.WriteLine("all args:");
            //foreach (string s in args)
            //{
            //    System.Console.WriteLine(s);
            //}
        }

        static void runCommand(string command, string command_args)
        {
            //System.Console.WriteLine("inp newer than outp ");

            System.Console.WriteLine("running " + command + " with args:" + command_args);

            //Create process
            System.Diagnostics.Process pProcess = new System.Diagnostics.Process();

            //strCommand is path and file name of command to run
            pProcess.StartInfo.FileName = command;

            //strCommandParameters are parameters to pass to program
            pProcess.StartInfo.Arguments = command_args;

            pProcess.StartInfo.UseShellExecute = false;

            //Set output of program to be written to process output stream
            pProcess.StartInfo.RedirectStandardOutput = true;

            //Optional
            //pProcess.StartInfo.WorkingDirectory = strWorkingDirectory;

            //Start the process
            pProcess.Start();

            //Get program output
            string strOutput = pProcess.StandardOutput.ReadToEnd();

            //Wait for process to finish
            pProcess.WaitForExit();
        }
    }

}
