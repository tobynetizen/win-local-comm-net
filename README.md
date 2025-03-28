## > Local Communications Network
A console-based locally-hosted chat system for Windows 10 using MySQL Connector/C++

### Features
- [ ] Data streaming
- [x] Nickname registration
- [ ] Multiple connections
- [x] Programmatic database control - server only
- [ ] Terminal UI integration

### MySQL Class Reference
#### QueryCompleted
**Description:** A callback function for a successful database query.<br />
**Returns:** `void` <br />
**Parameters:** `const char* sQuery` <br />

#### QueryFailed
**Description:** A callback function for an unsuccessful database query.<br />
**Returns:** `void` <br />
**Parameters:** `MYSQL* hConn` <br />

#### CreateTable
**Description:** A function for creating table `tbName` with a column `colName`.<br />
**Returns:** `void` <br />
**Parameters:** `const std::string& tbName`, `const std::string& colName` <br />

#### DropTable
**Description:** A function for deleting table `tbName` from a database.<br />
**Returns:** `void` <br />
**Parameters:** `const std::string& tbName` <br />

#### InsertKey
**Description:** Insert data `idxData` into column `colName` in table `tbName`.<br />
**Returns:** `void` <br />
**Parameters:**`const std::string& tbName`, `const std::string& colName`, `const std::string& idxData` <br />

#### QueryTable
**Description:** Returns data at index `idxPos` in table `tbName`.<br />
**Returns:** `void` <br />
**Parameters:** `const std::string& tbName`, `const int& idxPos` <br />

