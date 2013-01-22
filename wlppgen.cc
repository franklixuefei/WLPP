// Starter code for CS241 assignments 9-11
//
// C++ translation by Simon Parent (Winter 2011),
// based on Java code by Ondrej Lhotak,
// which was based on Scheme code by Gord Cormack.
// Modified July 3, 2012 by Gareth Davies
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

// The set of terminal symbols in the WLPP grammar.
const char *terminals[] = {
    "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID",
    "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM",
    "PCT", "PLUS", "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI",
    "SLASH", "STAR", "WAIN", "WHILE", "AMP", "LBRACK", "RBRACK", "NEW",
    "DELETE", "NULL"
};
int isTerminal(const string &sym) {
    int idx;
    for(idx=0; idx<sizeof(terminals)/sizeof(char*); idx++)
        if(terminals[idx] == sym) return 1;
    return 0;
}

// Data structure for storing the parse tree.
class tree {
public:
    string rule;
    vector<string> tokens;
    vector<tree*> children;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};

// Call this to display an error message and exit the program.
void bail(const string &msg) {
    // You can also simply throw a string instead of using this function.
    throw string(msg);
}

// Read and return wlppi parse tree.
tree *readParse(const string &lhs) {
    // Read a line from standard input.
    string line;
    getline(cin, line);
    if(cin.fail())
        bail("ERROR: Unexpected end of file.");
    tree *ret = new tree();
    // Tokenize the line.
    stringstream ss;
    ss << line;
    while(!ss.eof()) {
        string token;
        ss >> token;
        if(token == "") continue;
        ret->tokens.push_back(token);
    }
    // Ensure that the rule is separated by single spaces.
    for(int idx=0; idx<ret->tokens.size(); idx++) {
        if(idx>0) ret->rule += " ";
        ret->rule += ret->tokens[idx];
    }
    // Recurse if lhs is a nonterminal.
    if(!isTerminal(lhs)) {
        for(int idx=1/*skip the lhs*/; idx<ret->tokens.size(); idx++) {
            ret->children.push_back(readParse(ret->tokens[idx]));
        }
    }
    return ret;
}

string type(tree* parseTree, map<string, pair<string, int> >symTable) {
    if (parseTree->tokens[0] == "NUM") return "int";
    else if (parseTree->tokens[0] == "NULL") return "int*";
    else if (parseTree->tokens[0] == "ID") {
        string ret = symTable[parseTree->tokens[1]].first;
        if (ret != "") return ret;
        else {
            string msg = "ERROR: "+ parseTree->tokens[1] + " has not been declared yet";
            bail(msg);
        }
        
    }
    else {
        if (parseTree->tokens[0] == "expr") {
            if (parseTree->tokens.size() == 4) {
                string lop = type(parseTree->children[0], symTable);
                string rop = type(parseTree->children[2], symTable);
                if (parseTree->tokens[2] == "PLUS") {
                    
                    if (lop == "int" && rop == "int") {
                        return "int";
                    } else if (lop == "int" && rop == "int*") {
                        return "int*";
                    } else if (lop == "int*" && rop == "int") {
                        return "int*";
                    } else {
                        string msg = "ERROR: + operator does not support int* and int* as operands";
                        bail(msg);
                    }
                } else if (parseTree->tokens[2] == "MINUS") {
                    if (lop == "int" && rop == "int") {
                        return "int";
                    } else if (lop == "int*" && rop == "int*") {
                        return "int";
                    } else if (lop == "int*" && rop == "int") {
                        return "int*";
                    } else {
                        string msg = "ERROR: - operator does not support int and int* as operands";
                        bail(msg);
                    }
                    
                } else {
                    string msg = "ERROR: Impossible to get here";
                    bail(msg);
                }
            } else { // expr -> term
                return type(parseTree->children[0], symTable);
            }
        } else if (parseTree->tokens[0] == "term") {
            if (parseTree->tokens.size() == 4) {
                string lop = type(parseTree->children[0], symTable);
                string rop = type(parseTree->children[2], symTable);
                if (parseTree->tokens[2] == "STAR") {
                    
                    if (lop == "int" && rop == "int") {
                        return "int";
                    } else {
                        string msg = "ERROR: * operator only supports int and int as operands";
                        bail(msg);
                    }
                } else if (parseTree->tokens[2] == "SLASH") {
                    if (lop == "int" && rop == "int") {
                        return "int";
                    } else {
                        string msg = "ERROR: / operator only supports int and int as operands";
                        bail(msg);
                    }
                } else if (parseTree->tokens[2] == "PCT") {
                    if (lop == "int" && rop == "int") {
                        return "int";
                    } else {
                        string msg = "ERROR: % operator only supports int and int as operands";
                        bail(msg);
                    }
                    
                } else {
                    string msg = "ERROR: Impossible to get here";
                    bail(msg);
                }
            } else { // term -> factor
                return type(parseTree->children[0], symTable);
            }
            
        } else if (parseTree->tokens[0] == "factor") {
            if (parseTree->tokens.size() == 3) {
                string operand = type(parseTree->children[1], symTable);
                if (parseTree->tokens[1] == "AMP") {
                    if (operand == "int") return "int*";
                    else {
                        string msg = "ERROR: & can only be used before type int";
                        bail(msg);
                    }
                } else if (parseTree->tokens[1] == "STAR") {
                    if (operand == "int*") return "int";
                    else {
                        string msg = "ERROR: *(dereference) can only be used before type int*";
                        bail(msg);
                    }
                } else {
                    string msg = "ERROR: impossible to get here.";
                    bail(msg);
                }
            } else if (parseTree->tokens.size() == 4) {
                return type(parseTree->children[1], symTable);
            } else if (parseTree->tokens.size() == 6) {
                string operand = type(parseTree->children[3], symTable);
                if (operand == "int") return "int*";
                else {
                    string msg = "ERROR: only new int[int] is supported";
                    bail(msg);
                }
            } else { // factor -> {ID, NUM, NULL}
                return type(parseTree->children[0], symTable);
            }
        } else if (parseTree->tokens[0] == "lvalue") {
            if (parseTree->tokens.size() == 3) {
                string operand = type(parseTree->children[1], symTable);
                if (parseTree->tokens[1] == "STAR") {
                    if (operand == "int*") return "int";
                    else {
                        string msg = "ERROR: *(dereference) can only be used before type int*";
                        bail(msg);
                    }
                } else {
                    string msg = "ERROR: impossible to get here.";
                    bail(msg);
                }
            } else if (parseTree->tokens.size() == 4) {
                return type(parseTree->children[1], symTable);
            } else { // lvalue -> ID
                return type(parseTree->children[0], symTable);
            }
        } else {
            cout << "IMPOSSIBLE TO GET HERE" << endl;
            return ""; // impossible
            
        }
    }
}

void buildSymbolTableAndCheck(map<string, pair<string, int> >&symTable, tree* parseTree, int& counter) { 
    // 1. check for duplicated dcl; 2. record id derived by factor or lvalue
    if (parseTree == NULL) return;
    //cout << "TEST:" <<parseTree->tokens[0] << endl;
    if (parseTree->tokens[0] == "ID") {
        //cout << "ID entered" << endl; // test
        string stmtId = parseTree->tokens[1];
        map<string, pair<string, int> >::iterator it = symTable.find(stmtId);
        if (it != symTable.end()) {
            // nothing to do
        } else {
            string msg = "ERROR: " + stmtId + " has not been declared yet";
            bail(msg);
        }
        
    } else if (parseTree->tokens[0] == "dcl") {
        //cout << "dcl entered" << endl; // test
        tree* dclRulePtr = parseTree;
        tree* typePtr = dclRulePtr->children[0];
        tree* idPtr = dclRulePtr->children[1];
        map<string, pair<string, int> >::iterator it = symTable.find(idPtr->tokens[1]);
        if (typePtr->children.size() == 2) { // means its type is int*
            if (it != symTable.end()) {
                string msg = "ERROR: " + idPtr->tokens[1] + " has been declared as type of " + it->second.first + " before";
                bail(msg);
            } else {
                symTable[idPtr->tokens[1]] = make_pair("int*", counter);
                counter++;
            }
            
            
        } else { // means type is int
            if (it != symTable.end()) {
                string msg = "ERROR: " + idPtr->tokens[1] + " has been declared as type of " + it->second.first + " before";
                bail(msg);
            } else {
                symTable[idPtr->tokens[1]] = make_pair("int", counter);
                counter++;
            }
        }
        
    } else if (parseTree->tokens[0] == "expr" || parseTree->tokens[0] == "lvalue") {
        string result = type(parseTree, symTable);
    } else if (parseTree->tokens[0] == "test") {
        string type1 = type(parseTree->children[0], symTable);
        string type2 = type(parseTree->children[2], symTable);
        if (type1 == type2) {
            // do nothing
        } else {
            string msg = "ERROR: cannot compare " + type1 + " with " + type2;
            bail(msg);
        }
    } else if (parseTree->tokens[0] == "statement" && (parseTree->tokens.size() == 5 || parseTree->tokens.size() == 6)) {
        if (parseTree->tokens.size() == 5) {
            string type1 = type(parseTree->children[0], symTable);
            string type2 = type(parseTree->children[2], symTable);
            if (type1 == type2) {
                //do nothing
            } else {
                string msg = "ERROR: cannot assign value of type " + type2 + " to variable of type "+ type1;
                bail(msg);
            }
        } else { // size = 6
            if (parseTree->tokens[1] == "DELETE") {
                if (type(parseTree->children[3], symTable) == "int*") {
                    // do nothing
                } else {
                    string msg = "ERROR: delete can only be used with type int*";
                    bail(msg);
                }
            } else { // PRINTLN
                if (type(parseTree->children[2], symTable) == "int") {
                    // do nothing
                } else {
                    string msg = "ERROR: println can only be used with type int";
                    bail(msg);
                }
            }
        }
    } else if (parseTree->tokens[0] == "dcls") {
        
        if (parseTree->tokens.size() == 1) {
            return;
        } else {
            buildSymbolTableAndCheck(symTable, parseTree->children[0], counter);
            buildSymbolTableAndCheck(symTable, parseTree->children[1], counter);
            if (parseTree->tokens[4] == "NUM") {
                //cout << parseTree->children[1]->children[0]->tokens.size() << endl;
                if (parseTree->children[1]->children[0]->tokens.size() == 2) { // means int
                    // do nothing
                } else {
                    string msg = "ERROR: type mismatch: lvalue type should be int";
                    bail(msg);
                }
            } else { // NULL
                if (parseTree->children[1]->children[0]->tokens.size() == 3) { // means int*
                    // do nothing
                } else {
                    string msg = "ERROR: type mismatch: lvalue type should be int*";
                    bail(msg);
                }
            }
        }
    } else if (parseTree->tokens[0] == "procedure" && parseTree->children[5]->children[0]->tokens.size()==3) {
        string msg="ERROR: second argument of wain can only be of type int";
        bail(msg);
        
    } else {
        for (int i = 0; i < parseTree->children.size(); ++i) {
            if (parseTree->tokens.size() == 15) { // the procedure line
                if (parseTree->tokens[i+1] == "expr") {
                    if (type(parseTree->children[i], symTable) != "int") {
                        string msg="ERROR: return type can only be int";
                        bail(msg);
                    } else {
                        buildSymbolTableAndCheck(symTable, parseTree->children[i], counter);
                    }
                } else {
                    buildSymbolTableAndCheck(symTable, parseTree->children[i], counter);
                }
            } else {
                buildSymbolTableAndCheck(symTable, parseTree->children[i], counter);
            }
            
        }
    }
    
}

tree *parseTree;

// Compute symbols defined in t.
void genSymbols(tree *t) {
}

//void pushOneToTenToMem (map<string, int>numList) { //<number, pos>
//    for (int i = 0; i < 10; ++i) {
//        ostringstream oss;
//        oss << i;
//        numList.insert(make_pair((string)oss.str(), i));
//        oss.flush();
//    }
//}

void push(string reg) {
    cout << "sw " << reg << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

void pop(string reg) {
    cout << "add $30, $30, $4" << endl;
    cout << "lw " << reg << ", -4($30)" << endl;
}

void initialize() {
    cout << ".import print" << endl;
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "sw $31, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;

    
    cout << "sw $1, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    
    cout << "sw $2, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    
    cout << "add $29, $30, $0" << endl;
    
    
    cout << "lis $6" << endl;
    cout << ".word push" << endl;
    
    cout << "lis $7" << endl;
    cout << ".word pop" << endl;

    
    cout << "lis $8" << endl;
    cout << ".word print" << endl;
    
    
    
    cout << "lis $9" << endl;
    cout << ".word new" << endl;

    
    cout << "lis $14" << endl;
    cout << ".word delete" << endl;

    
}

bool specialLvalue = false;

// Generate the code for the parse tree t.
void genCode(tree *t, map<string, pair<string, int> >symTable, map<string, pair<string, int> >&varStack, int &counter, int &whileCounter, int &ifCounter, bool &statements_entered, bool &initCalled) {
    if (t == NULL) return;
    else if (t->tokens[0] == "ID") {
        int symPos = symTable[t->tokens[1]].second;
        if (symPos == 0 || symPos == 1) {
            cout << "lw $3, ";
            if (symPos == 0) cout << "4($29)" << endl;
            else cout << "0($29)" << endl;
        } else {
            symPos = varStack[t->tokens[1]].second;
            cout << "lw $3, " << -4-symPos*4 << "($29)" << endl;
        }
    } else if (t->tokens[0] == "NUM") {
        cout << "lis $3" << endl;
        cout << ".word " << t->tokens[1] << endl;
    } else if (t->tokens[0] == "NULL") { // set $3 to null addr
        cout << "add $3, $0, $0" << endl;
    } else if (t->tokens[0] == "statements") {
        if (!statements_entered) { // do the following when first time entering statements
            statements_entered = true;
            cout << "lis $20" << endl;
            cout << ".word " << counter*4 << endl;
            cout << "sub $30, $30, $20" << endl;
        }
        if (t->tokens.size() == 1) { // statements ->
            return;
        } else { // statements -> statements statement
            genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            genCode(t->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            
        }
        
    } else if (t->tokens[0] == "dcls") {
        if (t->children.size() != 0) {
            genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            int varType = t->children[1]->children[0]->children.size();
            if (varType == 1) {
                varStack[t->children[1]->children[1]->tokens[1]] = make_pair("int", counter);
            } else {
                varStack[t->children[1]->children[1]->tokens[1]] = make_pair("int*", counter);
            }
            if (t->tokens[4] == "NUM") {
                // store to stack
                // sw to mem
                                
                cout << "lis $3" << endl;
                cout << ".word " << t->children[3]->tokens[1] << endl;
                counter++;
                cout << "sw $3, "<< -4*counter << "($30)" << endl;
            } else { // NULL
                //cout << "add $3, $0, $0" << endl;
                counter++;
                cout << "sw $0, "<< -4*counter << "($30)" << endl;
            }
            
        }

    } else if (t->tokens[0] == "expr") {
        if (t->children.size() == 1) {
            genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
        } else {
            // size == 3
            string f = type(t->children[0], symTable);
            string s = type(t->children[2], symTable);
            if (f == "int" && s == "int") {
                genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "jalr $6" << endl; // push
                genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "jalr $7" << endl; // pop
                if (t->tokens[2] == "PLUS") {
                    cout << "add $3, $3, $5" << endl;
                } else { // MINUS
                    cout << "sub $3, $5, $3" << endl;
                }

            } else if (f == "int*" && s == "int") {
                genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "jalr $6" << endl; // push
                genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
                cout << "jalr $7" << endl; // pop
                if (t->tokens[2] == "PLUS") {
                    cout << "add $3, $3, $5" << endl;
                } else { // MINUS
                    cout << "sub $3, $5, $3" << endl;
                }
            } else if (f == "int" && s == "int*") {
                genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
                cout << "jalr $6" << endl; // push
                genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "jalr $7" << endl; // pop
                cout << "add $3, $5, $3" << endl;
                
            } else if (f == "int*" && s == "int*") {
                genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "jalr $6" << endl; // push
                genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
                cout << "jalr $7" << endl; // pop
                cout << "sub $3, $5, $3" << endl;
                cout << "div $3, $4" << endl;
                cout << "mflo $3" << endl;
            }
            
            
        }
    } else if (t->tokens[0] == "term") {
        if (t->children.size() == 1) {
            genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
        } else {
            // size == 3
            genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "jalr $6" << endl; // push
            genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "jalr $7" << endl; // pop
            if (t->tokens[2] == "STAR") {
                cout << "mult $3, $5" << endl;
                cout << "mflo $3" << endl;
            } else if (t->tokens[2] == "SLASH") { // MINUS
                cout << "div $5, $3" << endl;
                cout << "mflo $3" << endl;
            } else  { // PCT
                cout << "div $5, $3" << endl;
                cout << "mfhi $3" << endl;
            }
            
        }
    } else if (t->tokens[0] == "factor") {
        if (t->children.size() == 1) { // factor -> ID, factor -> NUM, factor -> NULL
            genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
        } else if (t->tokens[1] == "LPAREN") {
            genCode(t->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
        } else if (t->tokens[1] == "AMP") {
            string lvalueDeri = t->children[1]->tokens[1];
            
            if (lvalueDeri == "ID") {
                string id = t->children[1]->children[0]->tokens[1];
                int symPos = symTable[id].second;
                if (symPos == 0 || symPos == 1) {
                    if (symPos == 0) {
                        cout << "lis $3" << endl;
                        cout << ".word 4" << endl;
                    }
                    else cout << "add $3, $0, $0" << endl;
                } else {
                    symPos = varStack[id].second;
                    cout << "lis $3" << endl;
                    cout << ".word " << -4-symPos*4 << endl;
                }
                cout << "add $3, $29, $3" << endl;
            } else if (lvalueDeri == "STAR") {
                genCode(t->children[1]->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            } else { // LAPREN
		specialLvalue = true;
                genCode(t->children[1]->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            }
        } else if (t->tokens[1] == "STAR") {
            genCode(t->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "lw $3, 0($3)" << endl;
        } else if (t->tokens[1] == "NEW") {
            if (!initCalled) {
                cout << "lis $10" << endl;
                cout << ".word init" << endl;
                cout << "jalr $10" << endl; // consumes $2
                initCalled = true;
            }
            
            genCode(t->children[3], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "add $1, $3, $0" << endl;
            cout << "jalr $9" << endl;
        }
    } else if (t->tokens[0] == "lvalue") {
       if (!specialLvalue) { 
        if (t->children.size() == 1) { // lavlue -> ID
            string symbol = t->children[0]->tokens[1];
            int symPos = symTable[symbol].second;
            if (symPos == 0 || symPos == 1) {
                if (symPos == 0) {
                    cout << "sw $3, 4($29)" << endl;
                } else {
                    cout << "sw $3, 0($29)" << endl;
                }
            } else {
                int pos = varStack[symbol].second;
                cout << "sw $3, " << -4-pos*4 << "($29)" << endl;
            }
            
        } else if (t->children.size() == 2) { // lvalue -> STAR factor
            push("$3");
            genCode(t->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            pop("$5");
            cout << "sw $5, 0($3)" << endl;
            
        } else { // lvalue -> ( lvalue )
            genCode(t->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
        }
       } else {
	    specialLvalue = false;
	    string lvalueDeri = t->tokens[1];

            if (lvalueDeri == "ID") {
                string id = t->children[0]->tokens[1];
                int symPos = symTable[id].second;
                if (symPos == 0 || symPos == 1) {
                    if (symPos == 0) {
                        cout << "lis $3" << endl;
                        cout << ".word 4" << endl;
                    }
                    else cout << "add $3, $0, $0" << endl;
                } else {
                    symPos = varStack[id].second;
                    cout << "lis $3" << endl;
                    cout << ".word " << -4-symPos*4 << endl;
                }
                cout << "add $3, $29, $3" << endl;
            } else if (lvalueDeri == "STAR") {
                genCode(t->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            } else { // LAPREN
                specialLvalue = true;
                genCode(t->children[1], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            }

       }
    } else if (t->tokens[0] == "statement") {
        if (t->tokens[1] == "PRINTLN") {
            genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "add $1, $3, $0" << endl;
            cout << "jalr $8" << endl;
        } else if (t->tokens[1] == "lvalue") {
            genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled); // now $3 has been set to the result of expr.
            genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            
        } else if (t->tokens[1] == "WHILE") {
            whileCounter++;
            cout << "loop" << whileCounter << ":" << endl;
            
            genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "bne $3, $11, done" << whileCounter << endl;
            int tempWhileCounter = whileCounter;
            genCode(t->children[5], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "beq $0, $0, loop" << tempWhileCounter << endl;
            cout << "done" << tempWhileCounter << ":" << endl;
            cout << "add $0, $0, $0" << endl;
            
        } else if (t->tokens[1] == "IF") {
            ifCounter++;
            
            genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "bne $3, $11, else" << ifCounter << endl;
            int tempifCounter = ifCounter;
            genCode(t->children[5], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "beq $0, $0, endif" << tempifCounter << endl;
            cout << "else" << tempifCounter << ":" << endl;
            genCode(t->children[9], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "endif" << tempifCounter << ":" << endl;
            cout << "add $0, $0, $0" << endl;
            
        } else if (t->tokens[1] == "DELETE") {
            if (!initCalled) {
                cout << "lis $10" << endl;
                cout << ".word init" << endl;
                cout << "jalr $10" << endl; // consumes $2
                initCalled = true;
            }
            genCode(t->children[3], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            cout << "add $1, $3, $0" << endl;
            cout << "jalr $14" << endl;
        }
    } else if (t->tokens[0] == "test") {
        genCode(t->children[0], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
        cout << "jalr $6" << endl; // push
        genCode(t->children[2], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
        cout << "jalr $7" << endl; // pop
        if (type(t->children[0], symTable) == "int") { // or t->children[2] == int
            if (t->tokens[2] == "LT") {
                cout << "slt $3, $5, $3" << endl;
            } else if (t->tokens[2] == "EQ") {
                cout << "slt $12, $3, $5" << endl;
                cout << "slt $13, $5, $3" << endl;
                cout << "add $3, $12, $13" << endl;
                cout << "sub $3, $11, $3" << endl;
            } else if (t->tokens[2] == "NE") {
                cout << "slt $12, $3, $5" << endl;
                cout << "slt $13, $5, $3" << endl;
                cout << "add $3, $12, $13" << endl;
            } else if (t->tokens[2] == "LE") {
                cout << "slt $3, $3, $5" << endl;
                cout << "sub $3, $11, $3" << endl;
            } else if (t->tokens[2] == "GE") {
                cout << "slt $3, $5, $3" << endl;
                cout << "sub $3, $11, $3" << endl;
            } else if (t->tokens[2] == "GT") {
                cout << "slt $3, $3, $5" << endl;
            }
        } else { // int*
            if (t->tokens[2] == "LT") {
                cout << "sltu $3, $5, $3" << endl;
            } else if (t->tokens[2] == "EQ") {
                cout << "sltu $12, $3, $5" << endl;
                cout << "sltu $13, $5, $3" << endl;
                cout << "add $3, $12, $13" << endl;
                cout << "sub $3, $11, $3" << endl;
            } else if (t->tokens[2] == "NE") {
                cout << "sltu $12, $3, $5" << endl;
                cout << "sltu $13, $5, $3" << endl;
                cout << "add $3, $12, $13" << endl;
            } else if (t->tokens[2] == "LE") {
                cout << "sltu $3, $3, $5" << endl;
                cout << "sub $3, $11, $3" << endl;
            } else if (t->tokens[2] == "GE") {
                cout << "sltu $3, $5, $3" << endl;
                cout << "sub $3, $11, $3" << endl;
            } else if (t->tokens[2] == "GT") {
                cout << "sltu $3, $3, $5" << endl;
            }
        }
        
    } else {
        if (t->tokens[0] == "procedure") {
            for (int i = 8; i < t->children.size(); ++i) {
                genCode(t->children[i], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            } 
        } else {
            for (int i = 0; i < t->children.size(); ++i) {
                genCode(t->children[i], symTable, varStack, counter, whileCounter, ifCounter, statements_entered, initCalled);
            }
            
        }
    }
    
}

int main() {
    // Main program.
    map<string, pair<string, int> >symTable;
    map<string, pair<string, int> >varStack;
    try {
        parseTree = readParse("S");
        int counter = 0;
        buildSymbolTableAndCheck(symTable, parseTree, counter);
        initialize();
        genSymbols(parseTree);
        int s_counter = 0;
        int while_counter = 0;
        int if_counter = 0;
        bool statements_entered = false;
        bool initCalled = false;
        genCode(parseTree, symTable, varStack, s_counter, while_counter, if_counter, statements_entered, initCalled);
        // restore $31
        //cout << "add $30, $30, $4" << endl;
        cout << "lw $31, 8($29)" << endl;
        cout << "jr $31" << endl;
        // push proc
        cout << "push:" << endl;
        push("$3");
        cout << "jr $31" << endl;
        // pop proc
        cout << "pop:" << endl;
        pop("$5");
        cout << "jr $31" << endl;
        
        //cout << symTable.size() << endl;
        map<string, pair<string, int> >::iterator it;
//                for (it = symTable.begin(); it != symTable.end(); ++it) {
//                    cerr << it->first << " " << it->second.first << " " << it->second.second << endl;
//                }
        
    } catch(string msg) {
        cerr << msg << endl;
        return 1;
    }
    
    if (parseTree) delete parseTree;
    return 0;
}

