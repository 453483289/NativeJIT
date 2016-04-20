### This documents how to add an opcode to NativeJIT when that opcode is already being generated by CodeGen.


#### UnitTest/

Add appropriate unit tests

#### inc/NativeJIT/ExpressionNodeFactoryDecls.h

Add declaration.

#### inc/NativeJIT/ExpressionNodeFactory.h

Add a template defition for ExpressionNodeFactory.

#### inc/NativeJIT/X64CodeGenerator.h

Add to OpCode enum.
Add to X64CodeGenerator (CodeBuffer) class if appropriate.

#### inc/NativeJIT/Nodes/

If adding a new node type, add new node type here and add file for new node type in src/NativeJIT/CMakeLists.txt
