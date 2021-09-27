For compilation you need to run these lines in terminal

gcc -g -c memory_leak_algorithm.c -o memory_leak_algorithm.o

gcc -g -c application.c -o application.o

gcc -g -o exe memory_leak_algorithm.o application.o

Run and see result:

./exe

The program will report and show the leaked objects in the database.
As additional information program shows all the databases used in code.
As a final result - set of a leaked objects shown in such way:


Dumping Leaked Objects
-----------------------------------------------------------------------------------------------------|
2   ptr = bd1780     | next = bd1750     | units = 1    | struct_name = unit_t     | is_root = FALSE |
-----------------------------------------------------------------------------------------------------|
