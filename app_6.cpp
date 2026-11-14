#include <bits/stdc++.h>
#include <windows.h>
using namespace std;

// -------------------------------------------------------------
// FLOYD–WARSHALL DATA
// -------------------------------------------------------------
const int INF = 1e9;

vector<string> fw_places;
unordered_map<string,int> fw_id;
vector<vector<int>> fw_distKm;
vector<vector<int>> fw_distTime;


// -------------------------------------------------------------
// CONSOLE COLOR HELPERS (WINDOWS)
// -------------------------------------------------------------
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    setColor(7); // Default white
}

// -------------------------------------------------------------
// FILE NAMES
// -------------------------------------------------------------
const string USERS_FILE = "users.txt";
const string DEST_FILE = "destinations.txt";
const string VISIT_FILE = "visited.txt";
const string ADMIN_FILE = "admin.txt";
const string RECO_FILE = "recommended.txt";
const string HOTELS_FILE = "hotels.txt";
const string PATHS_FILE = "paths.txt";
const string TRANSPORT_FILE = "transport.txt";
const string GRAPH_FILE = "graph.txt";
const string EXPENSE_FILE = "expenses.txt";
const string DISTANCE_FILE = "distance.txt";



// -------------------------------------------------------------
// SAFE stoi()
// -------------------------------------------------------------
int safe_stoi(const string &s, int def = 0) {
    try { return stoi(s); }
    catch (...) { return def; }
}

// -------------------------------------------------------------
// SMALL HELPERS
// -------------------------------------------------------------
string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

vector<string> split(const string &s, char delim) {
    vector<string> v;
    string cur;
    for (char c : s) {
        if (c == delim) { v.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    v.push_back(cur);
    return v;
}

string join(const vector<string> &v, char delim) {
    string s;
    for (size_t i=0;i<v.size();i++) {
        if (i) s.push_back(delim);
        s += v[i];
    }
    return s;
}

void build_floyd_graph() {
    ifstream fin(PATHS_FILE.c_str());
    string line;

    vector<tuple<string,string,int,int>> edges;

    while (getline(fin, line)) {
        auto p = split(line, '|');
        if (p.size() < 4) continue;

        string u = p[0];
        string v = p[1];
        int time = safe_stoi(p[2]);
        int cost = safe_stoi(p[3]);

        int km = time / 2; // simple approx (you can refine)

        edges.push_back(make_tuple(u, v, km, time));

        if (!fw_id.count(u)) {
            fw_id[u] = fw_places.size();
            fw_places.push_back(u);
        }
        if (!fw_id.count(v)) {
            fw_id[v] = fw_places.size();
            fw_places.push_back(v);
        }
    }
    fin.close();

    int n = fw_places.size();
    fw_distKm.assign(n, vector<int>(n, INF));
    fw_distTime.assign(n, vector<int>(n, INF));

    for (int i = 0; i < n; i++) {
        fw_distKm[i][i] = 0;
        fw_distTime[i][i] = 0;
    }

    for (auto &e : edges) {
        string u, v; int km, tm;
        tie(u, v, km, tm) = e;

        int i = fw_id[u];
        int j = fw_id[v];
        fw_distKm[i][j] = fw_distKm[j][i] = km;
        fw_distTime[i][j] = fw_distTime[j][i] = tm;
    }
}

void floyd_warshall() {
    int n = fw_places.size();
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (fw_distKm[i][k] + fw_distKm[k][j] < fw_distKm[i][j]) {
                    fw_distKm[i][j] = fw_distKm[i][k] + fw_distKm[k][j];
                    fw_distTime[i][j] = fw_distTime[i][k] + fw_distTime[k][j];
                }
}


string get_line(const string &msg) {
    setColor(14);
    cout << msg;
    resetColor();
    cout.flush();
    string x;
    getline(cin, x);
    return x;
}

int get_int(const string &msg) {
    while (true) {
        setColor(14);
        cout << msg;
        resetColor();
        cout.flush();
        string s;
        getline(cin, s);
        try { return stoi(s); }
        catch (...) { cout << "Enter valid number.\n"; }
    }
}

// Forward declaration
struct Destination;

struct UserSession {
    string current_location;
    string transport_type;
    int travel_time;
    int travel_cost;

    string username;
    string selected_city;
    Destination* selected_destination;
    string selected_hotel;
    int budget;
    bool hotel_booked;

    UserSession() {
        travel_time = 0;
        travel_cost = 0;
        selected_city = "";
        selected_destination = NULL;
        selected_hotel = "City Center";
        budget = 0;
        hotel_booked = false;
    }
};

bool is_strong_password(const string& password) {
    if (password.length() < 6)
        return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char c : password) {
        if (isupper(c)) hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}


// -------------------------------------------------------------
// USER STRUCT & FUNCTIONS
// -------------------------------------------------------------
struct User {
    string username, password, security_answer;

    string to_line() const {
        return username + "|" + password + "|" + security_answer;
    }
    static User from_line(const string &line) {
        auto f = split(line, '|');
        User u;
        u.username = f.size()>0 ? f[0] : "";
        u.password = f.size()>1 ? f[1] : "";
        u.security_answer = f.size()>2 ? f[2] : "";
        return u;
    }
};

vector<User> load_users() {
    vector<User> v;
    ifstream fin(USERS_FILE.c_str());
    if (!fin.is_open()) {
        ofstream fout(USERS_FILE.c_str());
        return v;
    }
    string line;
    while (getline(fin, line)) {
        if (!trim(line).empty())
            v.push_back(User::from_line(line));
    }
    fin.close();
    return v;
}

void append_user(const User &u) {
    ofstream fout(USERS_FILE.c_str(), ios::app);
    fout << u.to_line() << "\n";
    fout.close();
}

void show_emergency_contacts() {
    cout << "\n══════════════════════════════════════════════════════════════\n";
    cout << "                      EMERGENCY CONTACTS                      \n";
    cout << "══════════════════════════════════════════════════════════════\n";

    cout << "\n🚑 MEDICAL EMERGENCY:\n";
    cout << "   • Emergency Ambulance: 108\n";
    cout << "   • Belagavi Government Hospital: 0831-242 2222\n";
    cout << "   • KLE Hospital: 0831-244 4444\n";
    cout << "   • Ashwin Hospital: 0831-240 1000\n";

    cout << "\n🚓 POLICE:\n";
    cout << "   • Police Emergency: 100\n";
    cout << "   • Women's Helpline: 1091\n";
    cout << "   • Traffic Police: 103\n";
    cout << "   • Belagavi Police Commissioner: 0831-240 3400\n";

    cout << "\n🚒 FIRE SERVICE:\n";
    cout << "   • Fire Emergency: 101\n";
    cout << "   • Belagavi Fire Station: 0831-242 2223\n";

    cout << "\n🆘 OTHER IMPORTANT NUMBERS:\n";
    cout << "   • Tourist Helpline: 1363\n";
    cout << "   • Roadside Assistance: 1073\n";
    cout << "   • Railway Enquiry: 139\n";
    cout << "   • Bus Stand Enquiry: 0831-240 2299\n";
    cout << "   • Airport Enquiry: 0831-240 8000\n";

    cout << "\n📍 NEAREST TOURIST POLICE STATIONS:\n";
    cout << "   • Belgaum Fort Tourist Police: 0831-240 3478\n";
    cout << "   • Gokak Falls Tourist Police: 08332-220 123\n";

    cout << "\n⚠️ IMPORTANT TIPS:\n";
    cout << "   1. Keep your hotel address and phone number handy\n";
    cout << "   2. Carry identification at all times\n";
    cout << "   3. Know the nearest hospital to your location\n";
    cout << "   4. Save emergency contacts in your phone\n";
    cout << "   5. In case of lost passport, contact your embassy\n";

    cout << "\n🏨 HOTEL EMERGENCY:\n";
    cout << "   • Inform your hotel reception immediately in any emergency\n";
    cout << "   • Most hotels have 24/7 medical assistance\n";
    cout << "   • Keep hotel emergency number with you\n";

    cout << "\n══════════════════════════════════════════════════════════════\n";
    cout << "   STAY SAFE! In case of emergency, don't panic. Call for help.\n";
    cout << "══════════════════════════════════════════════════════════════\n";

    cout << "\nPress Enter to continue...";
    cin.get();
}

// -------------------------------------------------------------
// HOTEL STRUCT & FUNCTIONS
// -------------------------------------------------------------
struct Hotel {
    string name;
    string city;
    double rating;
    int price_per_night;

    int total_rooms;
    int available_rooms;

    // Convert object → file line
    string to_line() const {
        return name + "|" +
               city + "|" +
               to_string(rating) + "|" +
               to_string(price_per_night) + "|" +
               to_string(total_rooms) + "|" +
               to_string(available_rooms);
    }

    // Convert file line → object
    static Hotel from_line(const string &line) {
        auto parts = split(line, '|');
        Hotel h;

        if (parts.size() > 0) h.name = parts[0];
        if (parts.size() > 1) h.city = parts[1];
        if (parts.size() > 2) h.rating = stod(parts[2]);
        if (parts.size() > 3) h.price_per_night = stoi(parts[3]);

        // NEW FIELDS (availability)
        if (parts.size() > 4) h.total_rooms = stoi(parts[4]);
        else h.total_rooms = 0;

        if (parts.size() > 5) h.available_rooms = stoi(parts[5]);
        else h.available_rooms = 0;

        return h;
    }
};


vector<Hotel> load_hotels() {
    vector<Hotel> hotels;
    ifstream fin(HOTELS_FILE.c_str());
    if (!fin.is_open()) {
        ofstream fout(HOTELS_FILE.c_str());
        fout << "Aura Hotel|Belagavi|4.1|2800|50|35\n";
        fout << "Blue Diamond|Belagavi|4.3|3200|40|25\n";
        fout << "Crystal Inn|Belagavi|3.9|2200|60|40\n";
        fout << "Dynasty Resort|Belagavi|4.0|3000|30|15\n";
        fout << "Elegance Hotel|Belagavi|4.2|2500|45|30\n";
        fout << "Fort View Lodge|Belagavi|4.5|3500|35|20\n";
        fout << "Garden Retreat|Belagavi|3.8|1800|55|35\n";
        fout << "Heritage Palace|Mumbai|4.8|8500|100|60\n";
        fout << "Imperial Suites|Mumbai|4.7|7500|120|70\n";
        fout << "Jewel Hotel|Bangalore|4.6|6500|80|45\n";
        fout << "Kings Court|Bangalore|4.9|9500|70|40\n";
        fout.close();
        fin.open(HOTELS_FILE.c_str());
    }

    string line;
    while (getline(fin, line)) {
        if (!trim(line).empty())
            hotels.push_back(Hotel::from_line(line));
    }
    fin.close();
    return hotels;
}

vector<Hotel> sort_hotels_by_price(vector<Hotel> hotels) {
    for (int i = 0; i < hotels.size() - 1; i++) {
        for (int j = 0; j < hotels.size() - i - 1; j++) {
            if (hotels[j].price_per_night > hotels[j+1].price_per_night) {
                swap(hotels[j], hotels[j+1]);
            }
        }
    }
    return hotels;
}


vector<Hotel> sort_hotels_by_rating(vector<Hotel> hotels) {
    int n = hotels.size();
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (hotels[j].rating < hotels[j+1].rating) {
                Hotel temp = hotels[j];
                hotels[j] = hotels[j+1];
                hotels[j+1] = temp;
            }
        }
    }
    return hotels;
}

// -------------------------------------------------------------
// DESTINATION STRUCT
// -------------------------------------------------------------
struct Destination {
    int id;
    string name, city, country, type;
    string history, best_time, entry_fee;
    vector<string> hotels, restaurants;
    bool recommended;

    Destination() {
        id = 0;
        recommended = false;
    }

    string to_line() const {
        return to_string(id) + "|" + name + "|" + city + "|" + country + "|" +
               type + "|" + history + "|" + best_time + "|" + entry_fee + "|" +
               join(hotels,';') + "|" + join(restaurants,';') + "|" +
               (recommended ? "1" : "0");
    }

    static Destination from_line(const string &line) {
        auto f = split(line, '|');
        Destination d;
        if (f.size()>0) d.id = safe_stoi(f[0]);
        if (f.size()>1) d.name = f[1];
        if (f.size()>2) d.city = f[2];
        if (f.size()>3) d.country = f[3];
        if (f.size()>4) d.type = f[4];
        if (f.size()>5) d.history = f[5];
        if (f.size()>6) d.best_time = f[6];
        if (f.size()>7) d.entry_fee = f[7];
        if (f.size()>8) d.hotels = split(f[8], ';');
        if (f.size()>9) d.restaurants = split(f[9], ';');
        if (f.size()>10) d.recommended = (f[10] == "1");
        return d;
    }
};

// -------------------------------------------------------------
// DESTINATION SORTING FUNCTIONS
// -------------------------------------------------------------
vector<Destination> sort_destinations_by_name(vector<Destination> destinations) {
    int n = destinations.size();
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (destinations[j].name > destinations[j+1].name) {
                swap(destinations[j], destinations[j+1]);
            }
        }
    }
    return destinations;
}

vector<Destination> sort_destinations_by_city(vector<Destination> destinations) {
    int n = destinations.size();
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (destinations[j].city > destinations[j+1].city) {
                swap(destinations[j], destinations[j+1]);
            }
        }
    }
    return destinations;
}

vector<Destination> sort_destinations_by_type(vector<Destination> destinations) {
    int n = destinations.size();
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (destinations[j].type > destinations[j+1].type) {
                swap(destinations[j], destinations[j+1]);
            }
        }
    }
    return destinations;
}

vector<Destination> load_destinations() {
    vector<Destination> v;
    ifstream fin(DEST_FILE.c_str());
    if (!fin.is_open()) {
        ofstream fout(DEST_FILE.c_str());
        // Belagavi Destinations (sorted by name)
        fout << "1|Belgaum Fort|Belagavi|India|Historical|Built in the 13th century by Ratta dynasty|October to March|Free entry|Fort View Lodge;Blue Diamond;Aura Hotel|Kamath Restaurant;Khanawal Restaurant|1\n";
        fout << "2|Gokak Falls|Belagavi|India|Natural|Waterfall on the Ghataprabha River with 52m drop|July to December|₹30|Dynasty Resort;Garden Retreat|Gokak Falls Cafe;River Restaurant|1\n";
        fout << "3|Godchinamalaki Falls|Belagavi|India|Natural|Also known as Markandeya Falls, beautiful cascade|Monsoon season|₹20|Forest Guest House;Nature Stay|Local Dhabas;Falls Restaurant|0\n";
        fout << "4|Kapileshwar Temple|Belagavi|India|Religious|Ancient Shiva temple carved from single rock|All year|Free entry|Aura Hotel;Crystal Inn|Temple Bhojanalaya;Prasad Point|1\n";
        fout << "5|Kittur Fort|Belagavi|India|Historical|Historic fort known for Kittur Rani Chennamma|November to February|₹50|Elegance Hotel;Fort View Lodge|Fort View Restaurant;Rani's Kitchen|1\n";

        // Bangalore Destinations
        fout << "6|Bangalore Palace|Bangalore|India|Historical|Built in 1874, Tudor-style architecture|September to March|₹230|Jewel Hotel;Kings Court|ITC Gardenia;Windmills Craftworks|1\n";
        fout << "7|Lalbagh Botanical Garden|Bangalore|India|Natural|240-acre garden with 1,854 plant species|All year|₹20|Kings Court;Jewel Hotel|Mavalli Tiffin Room;Third Wave Coffee|1\n";

        // Chennai Destinations
        fout << "8|Kapaleeshwarar Temple|Chennai|India|Religious|Dravidian architecture temple, 7th century|All year|Free entry|Park Hyatt;Leela Palace Chennai|Saravana Bhavan;Annalakshmi|0\n";
        fout << "9|Marina Beach|Chennai|India|Beach|Second longest urban beach in world|November to February|Free entry|Taj Coromandel;ITC Grand Chola|Murugan Idli Shop;Sandy's Chocolate Laboratory|1\n";

        // Delhi Destinations
        fout << "10|India Gate|Delhi|India|Historical|War memorial arch for Indian soldiers|October to March|Free entry|The Imperial;Taj Palace|Indian Accent;Bukhara|1\n";
        fout << "11|Red Fort|Delhi|India|Historical|UNESCO site built in 1639, Mughal architecture|October to March|₹35|Leela Palace Delhi;Oberoi New Delhi|Karim's;Paranthe Wali Gali|1\n";

        // Goa Destinations
        fout << "12|Basilica of Bom Jesus|Goa|India|Religious|UNESCO World Heritage Site|All year|Free entry|Cidade de Goa;Park Hyatt|Viva Goa;Fiesta|0\n";
        fout << "13|Calangute Beach|Goa|India|Beach|Most popular beach in Goa|October to May|Free entry|Taj Fort Aguada;Alila Diwa|Britto's;Infantaria|1\n";

        // Hyderabad Destinations
        fout << "14|Charminar|Hyderabad|India|Historical|Iconic monument built in 1591|October to March|₹20|Taj Falaknuma;Park Hyatt|Paradise;Shah Ghouse|1\n";
        fout << "15|Golconda Fort|Hyderabad|India|Historical|Fortress known for acoustic system|October to March|₹15|ITC Kohenur;Novotel|Bawarchi;Ohri's|1\n";

        // Jaipur Destinations
        fout << "16|Amber Fort|Jaipur|India|Historical|Majestic fort-palace built with marble|October to March|₹100|Rambagh Palace;Fairmont Jaipur|Suvarna Mahal;1135 AD|1\n";
        fout << "17|Hawa Mahal|Jaipur|India|Architectural|Palace of Winds with 953 windows|October to March|₹50|JW Marriott;Trident Jaipur|LMB;Niro's|1\n";

        // Kerala Destinations
        fout << "18|Backwaters|Kerala|India|Natural|Network of canals and lagoons|September to March|₹5000|Coconut Lagoon;Brunton Boatyard|Malabar Junction;Kayees|1\n";
        fout << "19|Munnar Tea Gardens|Kerala|India|Natural|Rolling hills covered with tea|September to May|Free entry|Windermere Estate;Tea County|Saravana Bhavan;Rapsy Restaurant|1\n";

        // Mumbai Destinations
        fout << "20|Gateway of India|Mumbai|India|Historical|Built in 1924, iconic arch monument|November to February|Free entry|Heritage Palace;Imperial Suites|Bade Miya;Leopold Cafe|1\n";
        fout << "21|Marine Drive|Mumbai|India|Scenic|3.6km long promenade along coast|October to March|Free entry|Oberoi Mumbai;Trident Nariman Point|Cafe Mondegar;Pizza by the Bay|1\n";
        fout.close();
        fin.open(DEST_FILE.c_str());
    }
    string line;
    while (getline(fin, line)) {
        if (!trim(line).empty())
            v.push_back(Destination::from_line(line));
    }
    fin.close();
    return v;
}

void save_all_destinations(const vector<Destination>& v) {
    ofstream fout(DEST_FILE.c_str(), ios::trunc);
    for (auto &d : v)
        fout << d.to_line() << "\n";
    fout.close();
}

// -------------------------------------------------------------
// VISITED
// -------------------------------------------------------------
vector<pair<string,int>> load_visited() {
    vector<pair<string,int>> v;
    ifstream fin(VISIT_FILE.c_str());
    if (!fin.is_open()) {
        ofstream fout(VISIT_FILE.c_str());
        return v;
    }
    string line;
    while (getline(fin, line)) {
        auto p = split(line, '|');
        if (p.size() < 2) continue;
        v.push_back(make_pair(p[0], safe_stoi(p[1])));
    }
    fin.close();
    return v;
}

void append_visited(const string &u, int id) {
    ofstream fout(VISIT_FILE.c_str(), ios::app);
    fout << u << "|" << id << "\n";
    fout.close();
}

// -------------------------------------------------------------
// TRANSPORT OPTIONS
// -------------------------------------------------------------
struct TransportOption {
    string type;
    int time;
    int cost;
    string comfort;

    bool operator<(const TransportOption& other) const {
        if (cost != other.cost) return cost < other.cost;
        return comfort > other.comfort;
    }
};

vector<TransportOption> get_transport_options(const string& from, const string& to) {
    vector<TransportOption> options;

    ifstream fin(TRANSPORT_FILE.c_str());
    string line;

    while (getline(fin, line)) {

        // Skip empty or garbage lines
        if (line.empty()) continue;

        auto parts = split(line, '|');

        // Minimum required:
        // From | To | Time | Cost | Transport
        if (parts.size() < 5) continue;

        string path_from = parts[0];
        string path_to   = parts[1];

        // Match both directions
        if (!((path_from == from && path_to == to) ||
              (path_from == to   && path_to == from)))
            continue;

        TransportOption opt;

        // SAFE stoi
        try {
            opt.time = stoi(parts[2]);
            opt.cost = stoi(parts[3]);
        } catch (...) {
            continue;   // skip malformed line safely
        }

        opt.type = parts[4];

        // Comfort handling
        if (parts.size() >= 6 && !parts[5].empty()) {
            opt.comfort = parts[5];
        } else {
            if (opt.type == "Cab")      opt.comfort = "High";
            else if (opt.type == "Auto") opt.comfort = "Medium";
            else                          opt.comfort = "Low";
        }

        options.push_back(opt);
    }

    fin.close();

    // Sort by cost (cheapest first)
    sort(options.begin(), options.end(),
         [](const TransportOption &a, const TransportOption &b) {
             return a.cost < b.cost;
         });

    return options;
}


// -------------------------------------------------------------
// STATE AND CITY SELECTION
// -------------------------------------------------------------
map<string, vector<string>> get_indian_states_cities() {
    map<string, vector<string>> states;
    states["Karnataka"] = {"Belagavi", "Bengaluru", "Mysuru", "Hubballi", "Mangaluru"};
    states["Maharashtra"] = {"Mumbai", "Pune", "Nagpur", "Nashik", "Aurangabad"};
    states["Goa"] = {"Panaji", "Margao", "Vasco da Gama", "Mapusa"};
    states["Tamil Nadu"] = {"Chennai", "Coimbatore", "Madurai", "Trichy", "Salem"};
    states["Delhi"] = {"New Delhi", "Delhi"};
    states["Rajasthan"] = {"Jaipur", "Udaipur", "Jodhpur", "Jaisalmer"};
    states["Kerala"] = {"Kochi", "Thiruvananthapuram", "Munnar", "Alleppey"};
    states["Telangana"] = {"Hyderabad", "Warangal"};
    return states;
}

// -------------------------------------------------------------
// MAP DISPLAY FUNCTIONS (C++ ONLY)
// -------------------------------------------------------------

void display_enhanced_route(const vector<string>& path_nodes,
                            const vector<int>& times,
                            const vector<string>& transport_types) {
    cout << "\n";
    cout << "================================================================\n";
    cout << "                        ROUTE MAP                               \n";
    cout << "================================================================\n\n";

    for (size_t i = 0; i < path_nodes.size(); i++) {
        cout << "  +---------------------------------------+\n";
        cout << "  |  [*] " << left << setw(32) << path_nodes[i] << "|\n";
        cout << "  +---------------------------------------+\n";

        if (i < path_nodes.size() - 1) {
            string transport_symbol;
            if (i < transport_types.size()) {
                if (transport_types[i] == "Cab") transport_symbol = "[CAB]";
                else if (transport_types[i] == "Bus") transport_symbol = "[BUS]";
                else if (transport_types[i] == "Auto") transport_symbol = "[AUTO]";
                else transport_symbol = "[---]";
            } else {
                transport_symbol = "[---]";
            }

            cout << "           |\n";
            cout << "           | " << transport_symbol;
            if (i < times.size()) {
                cout << " " << times[i] << " minutes";
            }
            cout << "\n";
            cout << "           |\n";
            cout << "           V\n";
        }
    }

    cout << "\n  >>> Journey Complete! <<<\n";
    cout << "================================================================\n\n";
}

void show_tour_route_map(
    const string& current,
    const string& hotel,
    const string& destination
) {
    int c = fw_id[current];
    int h = fw_id[hotel];
    int d = fw_id[destination];

    int d1 = fw_distKm[c][h];
    int t1 = fw_distTime[c][h];
    int d2 = fw_distKm[h][d];
    int t2 = fw_distTime[h][d];

    cout << "\n╔══════════════════════════════════════╗\n";
    cout << "║           TOUR ROUTE MAP             ║\n";
    cout << "╚══════════════════════════════════════╝\n\n";

    cout << "📍 CURRENT LOCATION\n";
    cout << "┌──────────────────────┐\n";
    cout << "│   " << setw(18) << left << current << "│\n";
    cout << "└──────────────────────┘\n";
    cout << "           │\n";
    cout << "           │  " << d1 << " km / " << t1 << " min\n";
    cout << "           ▼\n";

    cout << "🏨 NEAREST HOTEL\n";
    cout << "┌──────────────────────┐\n";
    cout << "│   " << setw(18) << left << hotel << "│\n";
    cout << "└──────────────────────┘\n";
    cout << "           │\n";
    cout << "           │  " << d2 << " km / " << t2 << " min\n";
    cout << "           ▼\n";

    cout << "🏁 DESTINATION\n";
    cout << "┌──────────────────────┐\n";
    cout << "│   " << setw(18) << left << destination << "│\n";
    cout << "└──────────────────────┘\n";

    cout << "\n════════════════════════════════════════\n";
    cout << "📏 TOTAL DISTANCE : " << d1 + d2 << " km\n";
    cout << "⏱ TOTAL TIME     : " << t1 + t2 << " minutes\n";
    cout << "════════════════════════════════════════\n";
}


void display_belagavi_city_map() {
    cout << "\n";
    cout << "================================================================\n";
    cout << "              BELAGAVI CITY - TOURIST MAP                       \n";
    cout << "================================================================\n\n";
    cout << "                         NORTH\n";
    cout << "                           ^\n";
    cout << "                           |\n";
    cout << "                           |\n";
    cout << "          +--------------------------------+\n";
    cout << "          |  Godchinamalaki Falls          |\n";
    cout << "          |  Type: Waterfall               |\n";
    cout << "          |  Distance: 40 km North         |\n";
    cout << "          +--------------------------------+\n";
    cout << "                       |\n";
    cout << "                       | 20 km\n";
    cout << "                       |\n";
    cout << "          +--------------------------------+\n";
    cout << "          |     Gokak Falls                |\n";
    cout << "          |     Type: Waterfall            |\n";
    cout << "          |     Distance: 60 km East       |\n";
    cout << "          +--------------------------------+\n";
    cout << "                       |\n";
    cout << "                       | 60 km\n";
    cout << "                       |\n";
    cout << "       [Hotel]----+----V----+----[Hotel]\n";
    cout << "                  |\n";
    cout << "       +----------+-----------+\n";
    cout << "       |                      |\n";
    cout << "       |   CITY CENTER        |\n";
    cout << "       |   (Belagavi)         |\n";
    cout << "       |   [Hotels Available] |\n";
    cout << "       +----------+-----------+\n";
    cout << "                  |\n";
    cout << "         +--------+--------+\n";
    cout << "         |                 |\n";
    cout << "         |                 |\n";
    cout << "    +----V-----+      +----V-----+\n";
    cout << "    | Belgaum  |      |  Kittur  |\n";
    cout << "    |   Fort   |      |   Fort   |\n";
    cout << "    | Distance:|      |Distance: |\n";
    cout << "    |   3 km   |      |  20 km   |\n";
    cout << "    | Type:    |      | Type:    |\n";
    cout << "    |Historical|      |Historical|\n";
    cout << "    +----------+      +----------+\n";
    cout << "\n";
    cout << "LEGEND:\n";
    cout << "  [*]  - Your Location\n";
    cout << "  [Hotel] - Hotel Locations\n";
    cout << "  +----+ - Tourist Destination\n";
    cout << "  |    - Connection/Road\n";
    cout << "\n";
    cout << "================================================================\n\n";
}

void display_grid_map(const string& start_location, const string& destination) {
    cout << "\n";
    cout << "================================================================\n";
    cout << "                    LOCATION GRID MAP                           \n";
    cout << "================================================================\n\n";

    const int width = 60;
    const int height = 20;
    vector<string> grid(height, string(width, ' '));

    for (int i = 0; i < height; i++) {
        grid[i][0] = '|';
        grid[i][width-1] = '|';
    }
    for (int j = 0; j < width; j++) {
        grid[0][j] = '-';
        grid[height-1][j] = '-';
    }
    grid[0][0] = '+'; grid[0][width-1] = '+';
    grid[height-1][0] = '+'; grid[height-1][width-1] = '+';

    map<string, pair<int,int>> positions;
    positions["City Center"] = make_pair(10, 30);
    positions["Belgaum Fort"] = make_pair(8, 25);
    positions["Kittur Fort"] = make_pair(12, 35);
    positions["Gokak Falls"] = make_pair(4, 30);
    positions["Godchinamalaki Falls"] = make_pair(2, 30);
    positions["Aura Hotel"] = make_pair(10, 28);
    positions["Blue Diamond"] = make_pair(10, 32);
    positions["Crystal Inn"] = make_pair(9, 30);

    if (positions.find(start_location) != positions.end()) {
        int y = positions[start_location].first;
        int x = positions[start_location].second;
        if (y >= 0 && y < height && x >= 0 && x < width-1) {
            grid[y][x] = 'S';
        }
    }

    if (positions.find(destination) != positions.end()) {
        int y = positions[destination].first;
        int x = positions[destination].second;
        if (y >= 0 && y < height && x >= 0 && x < width-1) {
            grid[y][x] = 'D';
        }
    }

    if (positions.find(start_location) != positions.end() &&
        positions.find(destination) != positions.end()) {
        int y1 = positions[start_location].first;
        int x1 = positions[start_location].second;
        int y2 = positions[destination].first;
        int x2 = positions[destination].second;

        for (int y = min(y1, y2); y <= max(y1, y2); y++) {
            if (grid[y][x1] == ' ') grid[y][x1] = '|';
        }
        for (int x = min(x1, x2); x <= max(x1, x2); x++) {
            if (grid[y2][x] == ' ') grid[y2][x] = '-';
        }
    }

    for (int i = 0; i < height; i++) {
        cout << "  " << grid[i] << "\n";
    }

    cout << "\n";
    cout << "  LEGEND:\n";
    cout << "    S = Start Location (" << start_location << ")\n";
    cout << "    D = Destination (" << destination << ")\n";
    cout << "    | - = Path\n";
    cout << "\n";
    cout << "================================================================\n\n";
}

void display_distance_matrix() {
    cout << "\n";
    cout << "================================================================\n";
    cout << "              DISTANCE MATRIX (in kilometers)                   \n";
    cout << "================================================================\n\n";

    vector<string> locations = {"City Center", "Belgaum Fort", "Kittur Fort",
                                "Gokak Falls", "Godchinamalaki"};

    int distances[5][5] = {
        {0,  3,  20, 60, 80},
        {3,  0,  22, 62, 82},
        {20, 22, 0,  70, 90},
        {60, 62, 70, 0,  20},
        {80, 82, 90, 20, 0}
    };

    cout << setw(18) << " ";
    for (size_t i = 0; i < locations.size(); i++) {
        cout << setw(15) << locations[i].substr(0, 12);
    }
    cout << "\n";
    cout << string(90, '-') << "\n";

    for (size_t i = 0; i < locations.size(); i++) {
        cout << setw(18) << left << locations[i];
        for (size_t j = 0; j < locations.size(); j++) {
            cout << setw(15) << right << distances[i][j];
        }
        cout << "\n";
    }

    cout << "\n================================================================\n\n";
}

void display_compass_map(const string& current_location) {
    cout << "\n";
    cout << "================================================================\n";
    cout << "                    COMPASS VIEW                                \n";
    cout << "================================================================\n\n";

    cout << "                      N\n";
    cout << "                      ^\n";
    cout << "                      |\n";
    cout << "          Godchinamalaki Falls\n";
    cout << "                 (80 km)\n";
    cout << "                      |\n";
    cout << "              Gokak Falls\n";
    cout << "                 (60 km)\n";
    cout << "                      |\n";
    cout << "  W <---- " << setw(15) << current_location << " ----> E\n";
    cout << "          (YOUR LOCATION)\n";
    cout << "                 |\n";
    cout << "            Belgaum Fort\n";
    cout << "              (3 km SW)\n";
    cout << "                 |\n";
    cout << "            Kittur Fort\n";
    cout << "              (20 km E)\n";
    cout << "                 |\n";
    cout << "                 v\n";
    cout << "                 S\n";
    cout << "\n";
    cout << "================================================================\n\n";
}

void show_map_menu(const string& start_location, const string& destination) {
    while (true) {
        cout << "\n";
        cout << "================================================================\n";
        cout << "                        MAP MENU                                \n";
        cout << "================================================================\n";
        cout << "1. View Route Map\n";
        cout << "2. View City Overview Map\n";
        cout << "3. View Grid Map\n";
        cout << "4. View Distance Matrix\n";
        cout << "5. View Compass Direction\n";
        cout << "6. Back to Trip Planning\n";
        cout << "================================================================\n";

        string choice = get_line("Choose map option (1-6): ");

        if (choice == "1") {
            vector<string> path = {start_location, destination};
            vector<int> times = {30};
            vector<string> transports = {"Cab"};
            display_enhanced_route(path, times, transports);
        }
        else if (choice == "2") {
            display_belagavi_city_map();
        }
        else if (choice == "3") {
            display_grid_map(start_location, destination);
        }
        else if (choice == "4") {
            display_distance_matrix();
        }
        else if (choice == "5") {
            display_compass_map(start_location);
        }
        else if (choice == "6") {
            break;
        }
        else {
            cout << "Invalid choice! Please select 1-6.\n";
        }

        cout << "\nPress Enter to continue...";
        cin.get();
    }
}

// -------------------------------------------------------------
// DESTINATION DETAILS DISPLAY
// -------------------------------------------------------------
void show_details(const Destination &d) {
    cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    setColor(11);
    cout << "║                    DESTINATION DETAILS                      ║\n";
    resetColor();
    cout << "╚════════════════════════════════════════════════════════════╝\n";
    setColor(10);
    cout << "  Name:        " << d.name << "\n";
    resetColor();
    cout << "  City:        " << d.city << "\n";
    cout << "  Country:     " << d.country << "\n";
    setColor(10);
    cout << "  Type:        " << d.type << "\n";
    resetColor();
    cout << "  History:     " << d.history << "\n";
    cout << "  Best Time:   " << d.best_time << "\n";
    cout << "  Entry Fee:   " << d.entry_fee << "\n";

    if (!d.hotels.empty() && trim(d.hotels[0]) != "") {
        cout << "  Nearby Hotels:\n";
        for (auto &h : d.hotels)
            if (!trim(h).empty()) cout << "    • " << h << "\n";
    }

    if (!d.restaurants.empty() && trim(d.restaurants[0]) != "") {
        cout << "  Nearby Restaurants:\n";
        for (auto &r : d.restaurants)
            if (!trim(r).empty()) cout << "    • " << r << "\n";
    }

    cout << "  Recommended: " << (d.recommended ? "✓ YES" : "✗ NO") << "\n";
    cout << "══════════════════════════════════════════════════════════════\n";
}

void update_hotel_availability(const string &hotel_name) {

    ifstream fin(HOTELS_FILE.c_str());
    vector<string> lines;
    string line;

    while (getline(fin, line)) {
        auto p = split(line, '|');

        if (p.size() >= 6 && p[0] == hotel_name) {
            int avail = stoi(p[5]);
            if (avail > 0) avail--;
            p[5] = to_string(avail);
            line = p[0]+"|"+p[1]+"|"+p[2]+"|"+p[3]+"|"+p[4]+"|"+p[5];
        }
        lines.push_back(line);
    }
    fin.close();

    ofstream fout(HOTELS_FILE.c_str());
    for (auto &l : lines)
        fout << l << "\n";
    fout.close();
}

void open_google_maps(const string& from,
                      const string& hotel,
                      const string& destination) {

    auto encode = [](string s) {
        for (size_t i = 0; i < s.length(); i++) {
            if (s[i] == ' ') s[i] = '+';
        }
        return s;
    };

    string url = "https://www.google.com/maps/dir/";
    url += encode(from + " Belagavi") + "/";
    url += encode(hotel + " Belagavi") + "/";
    url += encode(destination + " Belagavi");

#ifdef _WIN32
    string cmd = "start " + url;
#elif __APPLE__
    string cmd = "open " + url;
#else   // Linux
    string cmd = "xdg-open " + url;
#endif

    system(cmd.c_str());
}


// -------------------------------------------------------------
// TRIP PLANNING FUNCTION
// -------------------------------------------------------------
void plan_trip(UserSession &session) {

    string username = session.username;
    auto destinations = load_destinations();
    auto hotels = load_hotels();

    // ================= STEP 1: PRESENT LOCATION =================
    cout << "\nEnter your present location:\n";
    cout << "1. City Center\n";
    cout << "2. Aura Hotel\n";
    cout << "3. Crystal Inn\n";
    cout << "4. Blue Diamond\n";

    vector<string> locations = {
        "City Center", "Aura Hotel", "Crystal Inn", "Blue Diamond"
    };

    int loc;
    while (true) {
        loc = get_int("Choose (1-4): ");
        if (loc >= 1 && loc <= 4) break;
        cout << "Invalid choice!\n";
    }

    session.current_location = locations[loc - 1];
    cout << "Current Location: " << session.current_location << "\n";

    // ================= STEP 2: DESTINATION =================
    cout << "\nAvailable Destinations:\n";
    for (size_t i = 0; i < destinations.size(); i++) {
        cout << destinations[i].id << ". " << destinations[i].name << " (" << destinations[i].city << ")\n";
    }

    int dest_id;
    Destination* dest = NULL;

    while (true) {
        dest_id = get_int("Enter destination ID: ");
        for (size_t i = 0; i < destinations.size(); i++) {
            if (destinations[i].id == dest_id) {
                dest = &destinations[i];
                break;
            }
        }
        if (dest) break;
        cout << "Invalid ID!\n";
    }

    session.selected_destination = dest;
    session.selected_city = dest->city;

    // ================= STEP 3: HOTEL BOOKING =================
    string book = get_line("\nDo you want to book a hotel? (yes/no): ");
    string selected_hotel = session.current_location;
    bool hotel_booked = false;

    if (book == "yes" || book == "y") {

        vector<Hotel> city_hotels;
        for (size_t i = 0; i < hotels.size(); i++) {
            if (hotels[i].city == dest->city)
                city_hotels.push_back(hotels[i]);
        }

        if (!city_hotels.empty()) {

            cout << "\nSort hotels by:\n1. Price\n2. Rating\n";
            int sort_choice = get_int("Choose: ");

            if (sort_choice == 1)
                city_hotels = sort_hotels_by_price(city_hotels);
            else
                city_hotels = sort_hotels_by_rating(city_hotels);

            setColor(11);
            cout << "\nNearest Hotels:\n";
            resetColor();
            for (size_t i = 0; i < city_hotels.size(); i++) {
                Hotel &h = city_hotels[i];
                cout << i + 1 << ". " << h.name
                     << " | ₹" << h.price_per_night
                     << " | Rating: " << h.rating
                     << " | Rooms: " << h.available_rooms << "/" << h.total_rooms;

                if (h.available_rooms == 0)
                    cout << " ❌ FULL";
                else
                    cout << " ✅ AVAILABLE";

                cout << "\n";
            }

            int h_choice;
            while (true) {
                h_choice = get_int("Select hotel: ");
                if (h_choice < 1 || h_choice > (int)city_hotels.size()) {
                    cout << "Invalid choice!\n";
                    continue;
                }
                if (city_hotels[h_choice - 1].available_rooms == 0) {
                    cout << "❌ Hotel FULL. Choose another.\n";
                    continue;
                }
                break;
            }

            selected_hotel = city_hotels[h_choice - 1].name;
            hotel_booked = true;
            setColor(10);
            cout << "✓ Hotel Selected: " << selected_hotel << "\n";
            resetColor();
        }
    }

    session.selected_hotel = selected_hotel;
    session.hotel_booked = hotel_booked;

    // ================= STEP 4: TRANSPORT =================
    int total_time = 0;
    int total_cost = 0;
    string final_transport = "Not selected";

    // ---------- LEG 1: Present → Hotel ----------
    if (hotel_booked && session.current_location != selected_hotel) {

        cout << "\nTransport: " << session.current_location
             << " → " << selected_hotel << "\n";

        auto opt1 = get_transport_options(session.current_location, selected_hotel);

        if (!opt1.empty()) {
            cout << "1. Cheapest\n2. Comfortable\n";
            int c1 = get_int("Choose: ");

            TransportOption chosen = opt1[0];
            if (c1 == 2) {
                for (size_t i = 0; i < opt1.size(); i++) {
                    if (opt1[i].comfort == "High") {
                        chosen = opt1[i];
                        break;
                    }
                }
            }
            setColor(9);
            cout << "Selected: " << chosen.type
                 << " | Time: " << chosen.time
                 << " min | Cost: ₹" << chosen.cost << "\n";
            resetColor();
            total_time += chosen.time;
            total_cost += chosen.cost;
            final_transport = chosen.type;
        }
    }

    // ---------- LEG 2: Hotel / Present → Destination ----------
    string start_point = hotel_booked ? selected_hotel : session.current_location;

    cout << "\nTransport: " << start_point
         << " → " << dest->name << "\n";

    auto opt2 = get_transport_options(start_point, dest->name);

    if (opt2.empty() && start_point != "City Center") {
        cout << "No direct route found. Using City Center.\n";
        opt2 = get_transport_options("City Center", dest->name);
    }

    if (!opt2.empty()) {
        cout << "1. Cheapest\n2. Comfortable\n";
        int c2 = get_int("Choose: ");

        TransportOption chosen = opt2[0];
        if (c2 == 2) {
            for (size_t i = 0; i < opt2.size(); i++) {
                if (opt2[i].comfort == "High") {
                    chosen = opt2[i];
                    break;
                }
            }
        }

        cout << "Selected: " << chosen.type
             << " | Time: " << chosen.time
             << " min | Cost: ₹" << chosen.cost << "\n";

        total_time += chosen.time;
        total_cost += chosen.cost;
        final_transport = chosen.type;
    }

    session.transport_type = final_transport;
    session.travel_time = total_time;
    session.travel_cost = total_cost;

    cout << "\nTOTAL TRANSPORT SUMMARY\n";
    cout << "Total Time : " << total_time << " minutes\n";
    cout << "Total Cost : ₹" << total_cost << "\n";

    string hotel_node = hotel_booked ? selected_hotel : "City Center";

    show_tour_route_map(
        session.current_location,
        hotel_node,
        dest->name
    );

    string choice = get_line("\nOpen Google Maps? (y/n): ");
    if (choice == "y" || choice == "Y") {
        open_google_maps(
            session.current_location,
            hotel_node,
            dest->name
        );
    }


    // ================= STEP 5: RESTAURANTS =================
    cout << "\nRestaurants near " << dest->name << ":\n";
    for (size_t i = 0; i < dest->restaurants.size(); i++)
        cout << "• " << dest->restaurants[i] << "\n";

    // ================= STEP 6: CONFIRM =================
    string confirm = get_line("\nConfirm trip? (yes/no): ");
    if (confirm == "yes" || confirm == "y") {

        append_visited(username, dest_id);

        int hotel_cost = 0;
        if (hotel_booked) {
            for (size_t i = 0; i < hotels.size(); i++) {
                if (hotels[i].name == selected_hotel) {
                    hotel_cost = hotels[i].price_per_night;
                    break;
                }
            }
        }

        int final_cost = total_cost + hotel_cost;

        ofstream fout(EXPENSE_FILE.c_str(), ios::app);
        fout << username << "|" << dest_id << "|"
             << total_cost << "|" << hotel_cost << "|"
             << final_cost << "\n";
        fout.close();

        if (hotel_booked) {
            update_hotel_availability(selected_hotel);
        }
        setColor(10);
        cout << "🎉 Trip Confirmed!\n";
        resetColor();

        cout << "Total Cost: ₹" << final_cost << "\n";
    } else {
        cout << "\nTrip Cancelled.\n";
    }
}


void show_current_plan(const UserSession &session) {
    setColor(11);
    cout << "\n=========== CURRENT TRIP PLAN ===========\n";
    resetColor();
    cout << "User        : " << session.username << "\n";
    cout << "City        : "
         << (session.selected_city.empty() ? "Not selected" : session.selected_city) << "\n";
    setColor(10);
    cout << "Destination : "
         << (session.selected_destination ? session.selected_destination->name : "Not selected") << "\n";
    resetColor();
    cout << "Hotel       : " << session.selected_hotel << "\n";
    setColor(9);
    cout << "Transport   : "
         << (session.transport_type.empty() ? "Not selected" : session.transport_type) << "\n";
    resetColor();
    cout << "Travel Time : " << session.travel_time << " minutes\n";
    cout << "Travel Cost : ₹" << session.travel_cost << "\n";

    cout << "=========================================\n";
}


void plan_trip_by_budget(UserSession &session) {

    string username = session.username;

    auto destinations = load_destinations();
    auto hotels = load_hotels();
    setColor(11);
    cout << "\n========== PLAN TRIP BY BUDGET ==========\n";
    resetColor();

    int budget = get_int("Enter your total budget (₹): ");
    session.budget = budget;

    // ---------------- DESTINATION ----------------
    cout << "\nAvailable Destinations:\n";
    for (size_t i = 0; i < destinations.size(); i++) {
        cout << destinations[i].id << ". " << destinations[i].name << " (" << destinations[i].city << ")\n";
    }

    int dest_id;
    Destination* dest = NULL;

    while (true) {
        dest_id = get_int("Enter destination ID: ");
        for (size_t i = 0; i < destinations.size(); i++) {
            if (destinations[i].id == dest_id) {
                dest = &destinations[i];
                break;
            }
        }
        if (dest) break;
        cout << "Invalid ID!\n";
    }

    session.selected_city = dest->city;
    session.selected_destination = dest;

    // ---------------- TRANSPORT (CHEAPEST) ----------------
    auto transport_options =
        get_transport_options("City Center", dest->name);

    if (transport_options.empty()) {
        cout << "No transport data available.\n";
        return;
    }

    TransportOption cheapest_transport = transport_options[0];
    int transport_cost = cheapest_transport.cost;

    // ---------------- HOTEL (CHEAPEST) ----------------
    int hotel_cost = 0;
    string selected_hotel = "City Center";

    vector<Hotel> city_hotels;
    for (size_t i = 0; i < hotels.size(); i++) {
        if (hotels[i].city == dest->city)
            city_hotels.push_back(hotels[i]);
    }

    if (!city_hotels.empty()) {
        city_hotels = sort_hotels_by_price(city_hotels);
        selected_hotel = city_hotels[0].name;
        hotel_cost = city_hotels[0].price_per_night;
    }

    int total_cost = transport_cost + hotel_cost;

    // ---------------- SUMMARY ----------------
    cout << "\n========= BUDGET SUMMARY =========\n";
    cout << "Destination : " << dest->name << "\n";
    cout << "Transport   : " << cheapest_transport.type
         << " | ₹" << transport_cost << "\n";

    if (hotel_cost > 0) {
        cout << "Hotel       : " << selected_hotel
             << " | ₹" << hotel_cost << "\n";
    } else {
        cout << "Hotel       : Not booked\n";
    }

    cout << "---------------------------------\n";
    cout << "Total Cost  : ₹" << total_cost << "\n";
    cout << "Your Budget : ₹" << budget << "\n";

    // ---------------- CHECK BUDGET ----------------
    if (total_cost > budget) {
        setColor(12);
        cout << "\n❌ Budget insufficient.\n";
        resetColor();
        cout << "Extra required: ₹" << (total_cost - budget) << "\n";
        return;
    }

    // ---------------- CONFIRM ----------------
    string confirm = get_line("\nConfirm trip? (yes/no): ");
    if (confirm != "yes" && confirm != "y") {
        cout << "Trip cancelled.\n";
        return;
    }

    // ---------------- SAVE ----------------
    append_visited(username, dest_id);

    ofstream fout(EXPENSE_FILE.c_str(), ios::app);
    fout << username << "|"
         << dest_id << "|"
         << transport_cost << "|"
         << hotel_cost << "|"
         << total_cost << "\n";
    fout.close();

    // ---------------- UPDATE SESSION ----------------
    session.selected_hotel = selected_hotel;
    session.hotel_booked = (hotel_cost > 0);
    session.transport_type = cheapest_transport.type;
    session.travel_cost = transport_cost;
    session.travel_time = cheapest_transport.time;
    setColor(10);
    cout << "\n🎉 Trip planned successfully within budget!\n";
    resetColor();
}


void show_total_expenditure(const string &username) {

    ifstream fin(EXPENSE_FILE.c_str());
    if (!fin.is_open()) {
        cout << "\nNo expense records found.\n";
        return;
    }

    string line;
    int total = 0;
    bool found = false;

    cout << "\n========== EXPENDITURE DETAILS ==========\n";

    while (getline(fin, line)) {
        auto p = split(line, '|');
        if (p.size() < 5) continue;

        if (p[0] == username) {
            int dest_id = safe_stoi(p[1]);
            int transport = safe_stoi(p[2]);
            int hotel = safe_stoi(p[3]);
            int cost = safe_stoi(p[4]);

            cout << "Trip to Destination ID " << dest_id
                 << " | Transport ₹" << transport
                 << " | Hotel ₹" << hotel
                 << " | Total ₹" << cost << "\n";

            total += cost;
            found = true;
        }
    }

    fin.close();

    if (!found) {
        cout << "No trips found for user.\n";
    } else {
        cout << "----------------------------------------\n";
        cout << "TOTAL EXPENDITURE : ₹" << total << "\n";
    }

    cout << "========================================\n";
}

// -------------------------------------------------------------
// NAIVE PATTERN SEARCHING ALGORITHM
// -------------------------------------------------------------
bool patternSearch(const string &text, const string &pattern) {
    int n = text.length();
    int m = pattern.length();

    if (m == 0) return true;

    for (int i = 0; i <= n - m; i++) {
        int j;
        for (j = 0; j < m; j++) {
            if (tolower(text[i + j]) != tolower(pattern[j]))
                break;
        }
        if (j == m)
            return true;   // pattern found
    }
    return false;
}



void view_destinations_by_type() {

    auto destinations = load_destinations();

    // ----------- TAKE TYPE INPUT FROM USER -----------
    string user_type = get_line(
        "Enter destination type (Historical, Natural, Religious, etc.): "
    );

    if (user_type.empty()) {
        setColor(12);
        cout << "Type cannot be empty.\n";
        resetColor();
        return;
    }

    // ----------- OPTIONAL KEYWORD SEARCH -----------
    string keyword = get_line(
        "Enter keyword to search (press Enter to skip): "
    );

    setColor(11);
    cout << "\n===== SEARCH RESULTS =====\n";
    resetColor();

    bool found = false;

    for (size_t i = 0; i < destinations.size(); i++) {
        Destination &d = destinations[i];

        // Pattern search on TYPE
        if (patternSearch(d.type, user_type)) {

            // If keyword empty OR matches name/city
            if (keyword.empty() ||
                patternSearch(d.name, keyword) ||
                patternSearch(d.city, keyword)) {

                setColor(9);
                cout << d.id << " | " << d.name
                     << " | " << d.city
                     << " | " << d.type << "\n";
                resetColor();

                found = true;
            }
        }
    }

    if (!found) {
        setColor(12);
        cout << "No destinations found for this type.\n";
        resetColor();
    }
}


void view_complete_trip_history(const string &username) {

    auto destinations = load_destinations();
    ifstream fin(EXPENSE_FILE.c_str());

    if (!fin.is_open()) {
        cout << "\nNo trip history available.\n";
        return;
    }

    string line;
    bool found = false;
    setColor(11);
    cout << "\n========== COMPLETE TRIP HISTORY ==========\n";
    resetColor();

    while (getline(fin, line)) {
        auto p = split(line, '|');
        if (p.size() < 5) continue;

        if (p[0] != username) continue;

        int dest_id = safe_stoi(p[1]);
        int transport_cost = safe_stoi(p[2]);
        int hotel_cost = safe_stoi(p[3]);
        int total_cost = safe_stoi(p[4]);

        Destination *dest = NULL;
        for (size_t i = 0; i < destinations.size(); i++) {
            if (destinations[i].id == dest_id) {
                dest = &destinations[i];
                break;
            }
        }

        if (!dest) continue;

        found = true;

        cout << "\n------------------------------------------\n";
        setColor(9);
        cout << "Destination : " << dest->name << " (" << dest->city << ")\n";
        resetColor();
        cout << "Type        : " << dest->type << "\n";
        cout << "Start Point : City Center\n";

        if (hotel_cost > 0)
            cout << "Hotel Booked: Yes\n";
        else
            cout << "Hotel Booked: No\n";

        cout << "Transport   : ₹" << transport_cost << "\n";
        cout << "Hotel Cost  : ₹" << hotel_cost << "\n";
        setColor(10);
        cout << "Total Cost  : ₹" << total_cost << "\n";
        resetColor();
    }

    fin.close();

    if (!found) {
        cout << "No trips found for this user.\n";
    } else {
        cout << "\n==========================================\n";
    }
}

struct HotelDistance {
    Hotel hotel;
    int distance;
};

void insertion_sort_hotels(vector<HotelDistance> &v) {
    for (size_t i = 1; i < v.size(); i++) {
        HotelDistance key = v[i];
        int j = i - 1;

        while (j >= 0 && v[j].distance > key.distance) {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = key;
    }
}

struct RestaurantDistance {
    string name;
    int distance;
};

void selection_sort_restaurants(vector<RestaurantDistance> &v) {
    int n = v.size();
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (v[j].distance < v[min_idx].distance)
                min_idx = j;
        }
        swap(v[i], v[min_idx]);
    }
}


int get_distance_km(const string &place) {

    ifstream fin(DISTANCE_FILE.c_str());
    if (!fin.is_open())
        return 50; // fallback default

    string line;

    while (getline(fin, line)) {
        auto parts = split(line, '|');
        if (parts.size() != 2) continue;

        string place_name = parts[0];
        int dist = safe_stoi(parts[1], 50);

        if (place_name == place) {
            fin.close();
            return dist;
        }
    }

    fin.close();
    return 50; // default if not found
}

void view_nearest_hotels_and_restaurants() {

    auto destinations = load_destinations();
    auto hotels = load_hotels();

    int id = get_int("Enter destination ID: ");
    Destination *dest = NULL;

    for (size_t i = 0; i < destinations.size(); i++) {
        if (destinations[i].id == id) {
            dest = &destinations[i];
            break;
        }
    }

    if (!dest) {
        setColor(12);
        cout << "Invalid destination ID.\n";
        resetColor();
        return;
    }

    setColor(11);
    cout << "\n📍 Destination: " << dest->name << "\n";
    resetColor();

    // ================= HOTELS =================
    vector<HotelDistance> hotel_list;

    for (size_t i = 0; i < hotels.size(); i++) {
        if (hotels[i].city == dest->city && hotels[i].available_rooms > 0) {
            HotelDistance hd;
            hd.hotel = hotels[i];
            hd.distance = get_distance_km(dest->name);
            hotel_list.push_back(hd);
        }
    }

    insertion_sort_hotels(hotel_list);

    setColor(10);
    cout << "\n🏨 NEAREST HOTELS (Insertion Sort)\n";
    resetColor();

    for (size_t i = 0; i < hotel_list.size(); i++) {
        cout << hotel_list[i].hotel.name
             << " | Rooms: " << hotel_list[i].hotel.available_rooms
             << " | Distance: " << hotel_list[i].distance << " km\n";
    }

    // ================= RESTAURANTS =================
    vector<RestaurantDistance> rest_list;

    for (size_t i = 0; i < dest->restaurants.size(); i++) {
        RestaurantDistance rd;
        rd.name = dest->restaurants[i];
        rd.distance = rand() % 10 + 1;
        rest_list.push_back(rd);
    }

    selection_sort_restaurants(rest_list);

    setColor(9);
    cout << "\n🍽️ NEAREST RESTAURANTS (Selection Sort)\n";
    resetColor();

    for (size_t i = 0; i < rest_list.size(); i++) {
        cout << rest_list[i].name << " | Distance: " << rest_list[i].distance << " km\n";
    }
}


// -------------------------------------------------------------
// USER MENU
// -------------------------------------------------------------
void user_menu(const string &username) {
    UserSession session;
    session.username = username;

    auto ds = load_destinations();

    cout << "\nWelcome " << username << "!\n";
    cout << "══════════════════════════════════════════════════════════════\n";

    while (true) {
        cout << "\n══════════════════════════════════════════════════════════════\n";
        setColor(13); // Purple
        cout << "USER MENU\n";
        resetColor();
        cout << "══════════════════════════════════════════════════════════════\n";

        cout << "1. View All Destinations\n";
        cout << "2. View Recommended\n";
        cout << "3. View Destination Details\n";
        cout << "4. Mark as Visited\n";
        cout << "5. Plan a New Trip\n";
        cout << "6. Plan Trip by Budget\n";
        cout << "7. Show Current Trip Plan\n";
        cout << "8. Show Total Expenditure\n";
        cout << "9. View Destinations by Type\n";
        cout << "10. View My Trip History\n";
        cout << "11. Emergency contacts\n";
        cout << "12. View Nearest Hotels & Restaurants\n";
        cout << "13. Logout\n";
        cout << "Choose: ";

        string c;
        getline(cin, c);

        // -------------------------------------------------
        // 1. View All Destinations
        // -------------------------------------------------
        if (c == "1") {
            if (ds.empty()) {
                setColor(12); // Red
                cout << "No destinations available.\n";
                resetColor();
            } else {
                // Ask for sorting preference
                cout << "\nSort destinations by:\n";
                cout << "1. Name (A-Z)\n";
                cout << "2. City (A-Z)\n";
                cout << "3. Type (A-Z)\n";
                cout << "4. No sorting\n";
                string sort_choice = get_line("Choose (1-4): ");

                vector<Destination> sorted_ds = ds;

                if (sort_choice == "1") {
                    sorted_ds = sort_destinations_by_name(sorted_ds);
                    setColor(11); cout << "\nDestinations sorted by Name (A-Z):\n"; resetColor();
                } else if (sort_choice == "2") {
                    sorted_ds = sort_destinations_by_city(sorted_ds);
                    setColor(11); cout << "\nDestinations sorted by City (A-Z):\n"; resetColor();
                } else if (sort_choice == "3") {
                    sorted_ds = sort_destinations_by_type(sorted_ds);
                    setColor(11); cout << "\nDestinations sorted by Type (A-Z):\n"; resetColor();
                } else {
                    setColor(11); cout << "\nAll Destinations:\n"; resetColor();
                }

                cout << "ID | Name | City | Country | Type\n";
                cout << "-----------------------------------\n";

                for (size_t i = 0; i < sorted_ds.size(); i++) {
                    Destination &d = sorted_ds[i];
                    setColor(9); // Light blue
                    cout << d.id << " | " << d.name << " | "
                         << d.city << " | " << d.country << " | " << d.type << "\n";
                    resetColor();
                }
            }
        }

        // -------------------------------------------------
        // 2. View Recommended (NO SORTING)
        // -------------------------------------------------
        else if (c == "2") {
            bool found_rec = false;

            setColor(11);
            cout << "\nRecommended Destinations:\n";
            resetColor();

            if (ds.empty()) {
                setColor(12);
                cout << "No recommended destinations.\n";
                resetColor();
            } else {
                cout << "ID | Name | City | Country | Type\n";
                cout << "-----------------------------------\n";
                for (size_t i = 0; i < ds.size(); i++) {
                    if (ds[i].recommended) {
                        Destination &d = ds[i];
                        setColor(10); // Green for recommended
                        cout << d.id << " | " << d.name << " | "
                             << d.city << " | " << d.country << " | " << d.type << "\n";
                        resetColor();
                        found_rec = true;
                    }
                }

                if (!found_rec) {
                    setColor(12);
                    cout << "No recommended destinations.\n";
                    resetColor();
                }
            }
        }

        // -------------------------------------------------
        // 3. View Destination Details
        // -------------------------------------------------
        else if (c == "3") {
            int id = get_int("Enter destination ID: ");
            bool ok = false;

            for (size_t i = 0; i < ds.size(); i++) {
                if (ds[i].id == id) {
                    show_details(ds[i]);
                    ok = true;
                    break;
                }
            }

            if (!ok) {
                setColor(12);
                cout << "Destination not found.\n";
                resetColor();
            }
        }

        // -------------------------------------------------
        // 4. Mark as Visited
        // -------------------------------------------------
        else if (c == "4") {
            int id = get_int("Enter destination ID to mark as visited: ");
            bool dest_exists = false;

            for (size_t i = 0; i < ds.size(); i++) {
                if (ds[i].id == id) {
                    dest_exists = true;
                    break;
                }
            }

            if (dest_exists) {
                append_visited(username, id);
                setColor(10); // Green
                cout << "Marked as visited.\n";
                resetColor();
            } else {
                setColor(12);
                cout << "Destination ID not found.\n";
                resetColor();
            }
        }

        else if (c == "5") plan_trip(session);
        else if (c == "6") plan_trip_by_budget(session);
        else if (c == "7") show_current_plan(session);
        else if (c == "8") show_total_expenditure(username);
        else if (c == "9") view_destinations_by_type();
        else if (c == "10") view_complete_trip_history(username);
        else if (c == "11") show_emergency_contacts();
        else if (c == "12") view_nearest_hotels_and_restaurants();

        // -------------------------------------------------
        // 13. Logout
        // -------------------------------------------------
        else if (c == "13") {
            setColor(8); // Gray
            cout << "Logging out...\n";
            resetColor();
            return;
        }
        else {
            setColor(12);
            cout << "Invalid choice.\n";
            resetColor();
        }
    }
}

// -------------------------------------------------------------
// ADMIN PANEL
// -------------------------------------------------------------
pair<string,string> load_admin() {
    ifstream fin(ADMIN_FILE.c_str());
    if (!fin.is_open()) {
        ofstream fout(ADMIN_FILE.c_str());
        fout << "admin|admin123\n";
        fout.close();
        return make_pair("admin", "admin123");
    }

    string line;
    if (getline(fin, line)) {
        auto f = split(line, '|');
        if (f.size() >= 2) {
            fin.close();
            return make_pair(f[0], f[1]);
        }
    }
    fin.close();

    ofstream fout(ADMIN_FILE.c_str());
    fout << "admin|admin123\n";
    fout.close();
    return make_pair("admin", "admin123");
}

Destination* find_by_id(vector<Destination>& ds, int id) {
    for (size_t i = 0; i < ds.size(); i++)
        if (ds[i].id == id) return &ds[i];
    return NULL;
}

void manage_hotels() {

    auto hotels = load_hotels();

    if (hotels.empty()) {
        cout << "No hotels available.\n";
        return;
    }
    setColor(11);
    cout << "\n===== HOTEL LIST =====\n";
    resetColor();
    for (size_t i = 0; i < hotels.size(); i++) {
        cout << i + 1 << ". " << hotels[i].name
             << " | City: " << hotels[i].city
             << " | Price: ₹" << hotels[i].price_per_night
             << " | Rooms: " << hotels[i].available_rooms
             << "/" << hotels[i].total_rooms << "\n";
    }

    int choice = get_int("Select hotel to update (0 to cancel): ");
    if (choice == 0) return;

    if (choice < 1 || choice > (int)hotels.size()) {
        cout << "Invalid choice.\n";
        return;
    }

    Hotel &h = hotels[choice - 1];

    cout << "Updating hotel: " << h.name << "\n";

    h.price_per_night = get_int("New price per night: ");
    h.total_rooms = get_int("Total rooms: ");
    h.available_rooms = get_int("Available rooms: ");

    ofstream fout(HOTELS_FILE.c_str(), ios::trunc);
    for (size_t i = 0; i < hotels.size(); i++)
        fout << hotels[i].to_line() << "\n";
    fout.close();
    setColor(10);
    cout << "Hotel updated successfully.\n";
    resetColor();
}

void manage_transport_routes() {

    cout << "\n===== TRANSPORT ROUTES =====\n";

    ifstream fin(TRANSPORT_FILE.c_str());
    string line;
    int count = 1;

    while (getline(fin, line)) {
        if (!line.empty())
            cout << count++ << ". " << line << "\n";
    }
    fin.close();

    string add = get_line("\nAdd new route? (y/n): ");
    if (add != "y") return;

    string from = get_line("From: ");
    string to = get_line("To: ");
    int time = get_int("Time (minutes): ");
    int cost = get_int("Cost (₹): ");
    string type = get_line("Transport Type (Cab/Bus/Auto): ");

    ofstream fout(TRANSPORT_FILE.c_str(), ios::app);
    fout << from << "|" << to << "|" << time << "|" << cost << "|" << type << "\n";
    fout.close();

    cout << "Transport route added.\n";
}

void view_user_visit_reports() {

    auto visits = load_visited();
    auto destinations = load_destinations();

    if (visits.empty()) {
        cout << "No visit data available.\n";
        return;
    }

    map<int, int> visit_count;

    cout << "\n===== USER VISIT REPORT =====\n";

    for (size_t i = 0; i < visits.size(); i++) {
        cout << "User: " << visits[i].first << " visited Destination ID: " << visits[i].second << "\n";
        visit_count[visits[i].second]++;
    }

    cout << "\n===== MOST VISITED DESTINATIONS =====\n";
    for (map<int, int>::iterator it = visit_count.begin(); it != visit_count.end(); ++it) {
        for (size_t i = 0; i < destinations.size(); i++) {
            if (destinations[i].id == it->first) {
                cout << destinations[i].name << " → " << it->second << " visits\n";
                break;
            }
        }
    }
}

void view_expense_summary() {

    ifstream fin(EXPENSE_FILE.c_str());
    if (!fin.is_open()) {
        cout << "No expense data available.\n";
        return;
    }

    map<string, int> user_total;
    string line;

    while (getline(fin, line)) {
        auto p = split(line, '|');
        if (p.size() < 5) continue;

        string user = p[0];
        int cost = safe_stoi(p[4]);
        user_total[user] += cost;
    }
    fin.close();

    cout << "\n===== EXPENSE SUMMARY =====\n";
    int grand_total = 0;

    for (map<string, int>::iterator it = user_total.begin(); it != user_total.end(); ++it) {
        cout << "User: " << it->first << " | Total Spent: ₹" << it->second << "\n";
        grand_total += it->second;
    }

    cout << "-----------------------------\n";
    cout << "TOTAL SYSTEM REVENUE: ₹" << grand_total << "\n";
}

// -------------------------------------------------------------
// NEW FUNCTION: REMOVE DESTINATION
// -------------------------------------------------------------
void remove_destination() {
    auto ds = load_destinations();

    if (ds.empty()) {
        setColor(12);
        cout << "No destinations available to remove.\n";
        resetColor();
        return;
    }

    cout << "\n===== CURRENT DESTINATIONS =====\n";
    for (size_t i = 0; i < ds.size(); i++) {
        cout << ds[i].id << ". " << ds[i].name
             << " (" << ds[i].city << ") - " << ds[i].type;
        if (ds[i].recommended) cout << " [RECOMMENDED]";
        cout << "\n";
    }

    int id = get_int("\nEnter destination ID to remove (0 to cancel): ");
    if (id == 0) return;

    // Find destination
    int index = -1;
    string removed_name;

    for (size_t i = 0; i < ds.size(); i++) {
        if (ds[i].id == id) {
            index = i;
            removed_name = ds[i].name;
            break;
        }
    }

    if (index == -1) {
        setColor(12);
        cout << "Destination ID not found!\n";
        resetColor();
        return;
    }

    // Confirm removal
    cout << "\n⚠️ WARNING: This action cannot be undone!\n";
    string confirm = get_line("Are you sure you want to remove '" + removed_name + "'? (yes/no): ");
    if (confirm != "yes" && confirm != "y") {
        cout << "Removal cancelled.\n";
        return;
    }

    // Remove the destination
    ds.erase(ds.begin() + index);

    // Save updated list
    save_all_destinations(ds);

    setColor(10);
    cout << "✓ Destination '" << removed_name << "' removed successfully!\n";
    resetColor();
}

// -------------------------------------------------------------
// ADMIN MENU (UPDATED WITH REMOVE DESTINATION)
// -------------------------------------------------------------
void admin_menu() {
    auto ds = load_destinations();

    while (true) {
        cout << "\n--- ADMIN MENU ---\n";
        cout << "1. Add Destination\n";
        cout << "2. Edit Destination\n";
        cout << "3. Remove Destination\n";
        cout << "4. View All Destinations\n";
        cout << "5. Add to Recommended\n";
        cout << "6. Manage Hotels\n";
        cout << "7. Manage Transport Routes\n";
        cout << "8. View User Visit Reports\n";
        cout << "9. View Expense Summary\n";
        cout << "10. Logout\n";
        cout << "Choose option: ";

        string c;
        getline(cin, c);

        if (c == "1") {
            Destination d;

            int maxid = 0;
            for (size_t i = 0; i < ds.size(); i++) maxid = max(maxid, ds[i].id);
            d.id = maxid + 1;

            d.name = get_line("Name: ");
            d.city = get_line("City: ");
            d.country = get_line("Country: ");
            d.type = get_line("Type: ");
            d.history = get_line("History: ");
            d.best_time = get_line("Best time: ");
            d.entry_fee = get_line("Entry fee: ");
            d.hotels = split(get_line("Hotels (; separated): "), ';');
            d.restaurants = split(get_line("Restaurants (; separated): "), ';');
            d.recommended = (get_line("Recommended? (y/n): ") == "y");

            ds.push_back(d);
            save_all_destinations(ds);
            cout << "Destination added.\n";
        }
        else if (c == "2") {
            int id = get_int("Enter destination ID: ");
            Destination *d = find_by_id(ds, id);
            if (!d) { cout << "Not found.\n"; continue; }

            string t;
            t = get_line("Name [" + d->name + "]: "); if (!t.empty()) d->name = t;
            t = get_line("City [" + d->city + "]: "); if (!t.empty()) d->city = t;
            t = get_line("Country [" + d->country + "]: "); if (!t.empty()) d->country = t;
            t = get_line("Type [" + d->type + "]: "); if (!t.empty()) d->type = t;
            t = get_line("History [" + d->history + "]: "); if (!t.empty()) d->history = t;
            t = get_line("Best time [" + d->best_time + "]: "); if (!t.empty()) d->best_time = t;
            t = get_line("Entry fee [" + d->entry_fee + "]: "); if (!t.empty()) d->entry_fee = t;

            t = get_line("Hotels [" + join(d->hotels,';') + "]: ");
            if (!t.empty()) d->hotels = split(t, ';');

            t = get_line("Restaurants [" + join(d->restaurants,';') + "]: ");
            if (!t.empty()) d->restaurants = split(t, ';');

            string prompt = "Recommended (y/n) [" + string(d->recommended ? "y" : "n") + "]: ";
            t = get_line(prompt);
            if (!t.empty()) d->recommended = (t == "y");

            save_all_destinations(ds);
            cout << "Destination updated.\n";
        }
        else if (c == "3") {
            remove_destination();
            // Reload destinations after removal
            ds = load_destinations();
        }
        else if (c == "4") {
            if (ds.empty()) {
                cout << "No destinations available.\n";
            } else {
                cout << "\nID | Name | City | Country | Type | Status\n";
                cout << "-------------------------------------------\n";
                for (size_t i = 0; i < ds.size(); i++) {
                    Destination &d = ds[i];
                    cout << d.id << " | " << d.name << " | " << d.city << " | "
                         << d.country << " | " << d.type << " | " << (d.recommended ? "[RECOMMENDED]" : "") << "\n";
                }
            }
        }
        else if (c == "5") {
            int id = get_int("Enter ID to recommend: ");
            Destination *d = find_by_id(ds, id);
            if (!d) cout << "Not found.\n";
            else {
                ofstream fout(RECO_FILE.c_str(), ios::app);
                fout << d->id << "|" << d->name << "\n";
                fout.close();
                cout << "Added to recommended.txt\n";
            }
        }
        else if (c == "6") manage_hotels();
        else if (c == "7") manage_transport_routes();
        else if (c == "8") view_user_visit_reports();
        else if (c == "9") view_expense_summary();
        else if (c == "10") return;
        else cout << "Invalid choice.\n";
    }
}

// -------------------------------------------------------------
// FORGOT PASSWORD
// -------------------------------------------------------------
void forgot_password() {
    auto users = load_users();
    string uname = get_line("Enter username: ");

    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].username == uname) {
            string ans = get_line("Enter security answer: ");
            if (ans == users[i].security_answer) {
                string np;
                while (true) {
                    np = get_line("New password: ");
                    if (is_strong_password(np))
                        break;
                    cout << "❌ Weak password! Try again.\n";
                }
                users[i].password = np;
                ofstream fout(USERS_FILE.c_str(), ios::trunc);
                for (size_t j = 0; j < users.size(); j++)
                    fout << users[j].to_line() << "\n";
                fout.close();

                cout << "Password updated successfully.\n";
                return;
            }
            else {
                cout << "Wrong security answer.\n";
                return;
            }
        }
    }
    cout << "User not found.\n";
}


// -------------------------------------------------------------
// MAIN
// -------------------------------------------------------------
int main() {
    // Set console to UTF-8 for Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    build_floyd_graph();
    floyd_warshall();

    auto adminCreds = load_admin();
    setColor(11);
    cout << "══════════════════════════════════════════════════════════════\n";
    cout << "           TOURISM MANAGEMENT SYSTEM - BELAGAVI              \n";
    cout << "══════════════════════════════════════════════════════════════\n";
    resetColor();

    while (true) {
        cout << "\n══════════════════════════════════════════════════════════════\n";
        setColor(13);
        cout << "MAIN MENU\n";
        resetColor();
        cout << "══════════════════════════════════════════════════════════════\n";
        setColor(9);
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "3. Forgot Password\n";
        cout << "4. Admin Login\n";
        cout << "5. Exit\n";
        resetColor();
        cout << "Choose option: ";

        string c;
        getline(cin, c);

        if (c == "1") {
            string u = get_line("Username: ");
            cout << "\nPassword must contain:\n";
            cout << "- Minimum 6 characters\n";
            cout << "- At least 1 uppercase letter\n";
            cout << "- At least 1 lowercase letter\n";
            cout << "- At least 1 number\n";
            cout << "- At least 1 special character\n\n";

            string p;
            while (true) {
                p = get_line("Password: ");
                if (is_strong_password(p))
                    break;
                cout << "❌ Weak password! Please follow the rules above.\n";
            }
            string s = get_line("Security question (Favorite place): ");

            auto users = load_users();
            bool exists = false;
            for (size_t i = 0; i < users.size(); i++) {
                if (users[i].username == u) {
                    exists = true;
                    break;
                }
            }

            if (exists) {
                cout << "Username already exists.\n";
            } else {
                User new_user;
                new_user.username = u;
                new_user.password = p;
                new_user.security_answer = s;
                append_user(new_user);
                setColor(10);
                cout << "Registration successful!\n";
                resetColor();
            }
        }
        else if (c == "2") {
            auto users = load_users();
            string u = get_line("Username: ");
            string p = get_line("Password: ");
            bool ok = false;

            for (size_t i = 0; i < users.size(); i++) {
                if (users[i].username == u && users[i].password == p) {
                    ok = true;
                    break;
                }
            }

            if (ok) {
                user_menu(u);
            } else {
                cout << "Invalid username or password.\n";
            }
        }
        else if (c == "3") {
            forgot_password();
        }
        else if (c == "4") {
            string u = get_line("Admin username: ");
            string p = get_line("Admin password: ");
            if (u == adminCreds.first && p == adminCreds.second) {
                admin_menu();
            } else {
                cout << "Invalid admin credentials.\n";
            }
        }
        else if (c == "5") {
            cout << "\n══════════════════════════════════════════════════════════════\n";
            setColor(8);
            cout << "Thank you for using Tourism Management System. Goodbye!\n";
            resetColor();
            cout << "══════════════════════════════════════════════════════════════\n";
            break;
        }
        else {
            cout << "Invalid choice. Please enter 1-5.\n";
        }
    }

    return 0;
}
