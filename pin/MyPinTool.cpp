
#include <fstream>
#include <iostream>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <set>
#include "pin.H"

ofstream outFile;
bool ins_enable = false;

uint64_t total_bbls;
std::set<ADDRINT> known_bbls;
std::pair<std::set<ADDRINT>::iterator,bool> ret;

VOID docount(ADDRINT bbl) {
	total_bbls++;
	known_bbls.insert(bbl);
}

VOID start_inst()
{
	ins_enable = true;
}

VOID stop_inst()
{
	ins_enable = false;
}

VOID Routine(RTN rtn, VOID *v)
{
	AFUNPTR fnc = NULL;
	if (RTN_Name(rtn) == "CoreNewInstance") {
		fnc = start_inst;
	}

	if (RTN_Name(rtn) == "CoreDeleteInstance") {
		fnc = stop_inst;
	}

	if (fnc) {
		RTN_Open(rtn);

		// Insert a call at the entry point of a routine to know when to
		// start / stop tracing.
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)fnc, IARG_END);

		RTN_Close(rtn);
	}
}

// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID Trace(TRACE trace, VOID *v)
{
	if (ins_enable) {
		// Visit every basic block  in the trace
		for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
		{
			// Insert a call to docount into every bbl
			BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)docount, IARG_ADDRINT, BBL_Address(bbl), IARG_END);
		}
	}
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
	outFile << "{\"uniq_bb_count\": " << known_bbls.size() << ", ";
	outFile << "\"total_bb_count\": " << total_bbls << "}" << endl;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
	// Initialize symbol table code, needed for rtn instrumentation
	PIN_InitSymbols();

	outFile.open("bbcount.json");

	// Initialize pin
	if (PIN_Init(argc, argv)) return -1;

	RTN_AddInstrumentFunction(Routine, 0);
	TRACE_AddInstrumentFunction(Trace, 0);

	// Register Fini to be called when the application exits
	PIN_AddFiniFunction(Fini, 0);

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}
