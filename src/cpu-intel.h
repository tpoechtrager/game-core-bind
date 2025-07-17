// Known Intel models with total CCD (Tile) core/thread count
struct IntelCCDLayout {
  int cores;
  int threads;
  bool isEfficiency;
  bool isLowPowerEfficiency;
};

struct IntelModel {
  const char* name;
  std::array<IntelCCDLayout, 3> ccds; // Up to three core types (P-cores, E-cores, LP E-cores)
  int numCcds; // Number of active CCDs/Tiles
};

// Known Intel models since Alder Lake
// PS: This list took a lot of work to create. If you copy it, please at least give me some credit. ;)
static constexpr std::array<IntelModel, 98> Intel_KnownModels = {{
  // Alder Lake
  { "N95",             {{{ 4,  4, true, false }}},                     1 },
  { "N97",             {{{ 4,  4, true, false }}},                     1 },
  { "N100",            {{{ 4,  4, true, false }}},                     1 },
  { "N150",            {{{ 4,  4, true, false }}},                     1 },
  { "N200",            {{{ 4,  4, true, false }}},                     1 },
  { "N250",            {{{ 4,  4, true, false }}},                     1 },
  { "N300",            {{{ 8,  8, true, false }}},                     1 },
  { "N305",            {{{ 8,  8, true, false }}},                     1 },
  { "N350",            {{{ 8,  8, true, false }}},                     1 },
  { "N355",            {{{ 8,  8, true, false }}},                     1 },
  { "i3-N300",         {{{ 8,  8, true, false }}},                     1 },
  { "i3-N305",         {{{ 8,  8, true, false }}},                     1 },
  { "i3-N350",         {{{ 8,  8, true, false }}},                     1 },
  { "i3-N355",         {{{ 8,  8, true, false }}},                     1 },

  { "1210U",        {{{ 2,  4, false, false }, { 4,  4, true, false }}},        2 }, // 2P+4E 
  { "1215U",        {{{ 2,  4, false, false }, { 4,  4, true, false }}},        2 }, // 2P+4E
  { "1230U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1235U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1240U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1245U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1255U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1260U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1265U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E

  { "1220P",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1240P",        {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "1250P",        {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "1260P",        {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "1270P",        {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "1280P",        {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E

  { "1315U",        {{{ 2,  4, false, false }, { 4,  4, true, false }}},        2 }, // 2P+4E
  { "1335U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "1355U",        {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E

  { "13420H",       {{{ 4,  8, false, false }, { 4,  4, true, false }}},        2 }, // 4P+4E
  { "13500H",       {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "13700H",       {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E
  { "13900H",       {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E

  { "1340P",        {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "1350P",        {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "1360P",        {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "1370P",        {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E

  // Raptor Lake Refresh
  { "14400",        {{{ 6, 12, false, false }, { 4,  4, true, false }}},        2 }, // 6P+4E
  { "14500",        {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E
  { "14600",        {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E
  { "14700",        {{{ 8, 16, false, false }, { 12, 12, true, false }}},       2 }, // 8P+12E
  { "14900",        {{{ 8, 16, false, false }, { 16, 16, true, false }}},       2 }, // 8P+16E

  { "251",          {{{ 8, 16, false, false }, { 16,  16, true, false }}},      2 }, // 8P+16E

  { "210H",         {{{ 4,  8, false, false }, { 4,  4, true, false }}},        2 }, // 4P+4E
  { "220H",         {{{ 4,  8, false, false }, { 8,  8, true, false }}},        2 }, // 4P+8E
  { "240H",         {{{ 6, 12, false, false }, { 4,  4, true, false }}},        2 }, // 6P+4E
  { "250H",         {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E
  { "270H",         {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E

  { "160HL",        {{{ 6, 12, false, false }, { 8,  8, true, false }}},        2 }, // 6P+8E

  { "220U",         {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "250U",         {{{ 2,  4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E

  { "150UL",       {{{ 2, 4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E
  { "160UL",       {{{ 2, 4, false, false }, { 8,  8, true, false }}},        2 }, // 2P+8E

  // Meteor Lake
  { "125H",    {{{ 4,  8, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 4P+8E+2LPE
  { "135H",    {{{ 4,  8, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 4P+8E+2LPE
  { "155H",    {{{ 6, 12, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 6P+8E+2LPE
  { "165H",    {{{ 6, 12, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 6P+8E+2LPE
  { "185H",    {{{ 6, 12, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 6P+8E+2LPE

  { "125U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "135U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "155U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "165U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE

  { "134U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "164U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE

  { "125HL",   {{{ 4,  8, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 4P+8E+2LPE
  { "125UL",   {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "155UL",   {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE

  // Arrow Lake
  { "225U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "235U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "255U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE
  { "265U",    {{{ 2,  4, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 2P+8E+2LPE

  { "225H",    {{{ 4, 8, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 4P+8E+2LPE
  { "235H",    {{{ 4, 8, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 4P+8E+2LPE
  { "255H",    {{{ 6, 6, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 6P+8E+2LPE
  { "265H",    {{{ 6, 6, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 6P+8E+2LPE
  { "285H",    {{{ 6, 6, false, false }, { 8,  8, true, false }, { 2, 2, false, true }}}, 3 }, // 6P+8E+2LPE

  { "225",    {{{ 6, 6, false, false }, { 4,  4, true, false }}}, 2 }, // 6P+4E
  { "235",    {{{ 6, 6, false, false }, { 8,  8, true, false }}}, 2 }, // 6P+8E
  { "245",    {{{ 6, 6, false, false }, { 8,  8, true, false }}}, 2 }, // 6P+8E
  { "255",    {{{ 6, 6, false, false }, { 8,  8, true, false }}}, 2 }, // 6P+8E
  { "265",    {{{ 6, 6, false, false }, { 12,  12, true, false }}}, 2 }, // 8P+12E

  { "245HX",   {{{ 6,  6, false, false }, { 8,  8, true, false }}}, 2 }, // 6P+8E
  { "255HX",   {{{ 8,  8, false, false }, { 12, 12, true, false }}}, 2 }, // 8P+12E
  { "265HX",   {{{ 8,  8, false, false }, { 12, 12, true, false }}}, 2 }, // 8P+12E
  { "275HX",   {{{ 8,  8, false, false }, { 16, 16, true, false }}}, 2 }, // 8P+16E
  { "285HX",   {{{ 8,  8, false, false }, { 16, 16, true, false }}}, 2 }, // 8P+16E

  // Lunar Lake
  { "226V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "228V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "236V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "238V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "256V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "258V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "266V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "268V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
  { "288V",    {{{ 4, 4, false, false }, { 4,  4, false, true }}}, 2 }, // 4P+4LPE
}};

// Returns Intel model info for known CPUs.
// If multiple models match (e.g. "14900" and "14900HX"),
// the longest matching name is selected to ensure more specific variants (like HX) are preferred.
static const IntelModel* FindIntelModel(const std::string& brandStr) {
  const IntelModel* bestMatch = nullptr;
  size_t bestMatchLength = 0;

  for (const auto& entry : Intel_KnownModels) {
    const size_t nameLen = std::strlen(entry.name);
    if (nameLen <= bestMatchLength)
      continue;
    if (brandStr.find(entry.name) != std::string::npos) {
      bestMatch = &entry;
      bestMatchLength = nameLen;
    }
  }

  return bestMatch;
}
