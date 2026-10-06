/* Sim.cpp */
// Author: Programmer Intern Jordan (with help from my teammates! :P)

#include <iostream> // cout and stuff
#include <algorithm> // 
#include <vector> // for vectors
#include <fstream> // to output to the HTML file easier
#include <string> // due
#include <unordered_map> // to track the time points when there are X num of cars present

#include "Sim_Math.h" // poisson, binomial, percentage, crash_occurrence
#include "TimeCode.h"
#include "BigInteger.h" // to count the total number of cars, which may be a really large value


// Hard-code some time durations for ease of use, uie L to avoid overflow
#define DURATION_10MIN 600
#define DURATION_1HR 3600
#define DURATION_1DAY 86400L
#define DURATION_1WK 604800L
#define DURATION_1MO 2629746L
#define DURATION_3MO 7889238L
#define DURATION_6MO 15778476L
#define DURATION_1YRS 31556952L
#define DURATION_5YRS 157784760L
#define DURATION_10YRS 315569520L


// A struct is like an object without any functions
// These structs are used to keep some of the important data together
struct data_point_pair{ // the data plotted on the chart
	TimeCode t; // x-axis
	int num_cars; // y-axis
};

struct results{ // All the "output" data from the simulation
	TimeCode total_duration;
	int road_capacity;
	int new_car_rate;
	int num_crashes;
	std::string total_cars;
	std::unordered_map<int, std::vector<TimeCode>> count_times_map;

	std::vector<int> x_data;
	std::vector<int> y_data;
	TimeCode plotted_duration;
};


// Ideally this would be a function of the TimeCode object itself
std::string TimeCodeVerboseString(TimeCode tc){
	std::string res = std::to_string(tc.GetHours()) + "hrs " + std::to_string(tc.GetMinutes()) + "mins " + std::to_string(tc.GetSeconds()) + "secs";
	return res;
}


// Function to generate the HTML file with embedded Plotly.js chart
void generateHTMLoutput(const results& data, const std::string& filename = "plot.html") {

    std::ofstream html_file(filename);

    if (!html_file.is_open()) {
        std::cerr << "Error: Could not open " << filename << " for writing." << std::endl;
        return;
    }

    // Start of the HTML boilerplate
    html_file << "<!DOCTYPE html>\n";
    html_file << "<html>\n";
    html_file << "<head>\n";
    html_file << "    <title>Traffic Simulation</title>\n";
    // Load Plotly.js library from a CDN, requires internet access
    html_file << "    <script src=\"https://cdn.plot.ly/plotly-latest.min.js\"></script>\n";
    html_file << "    <style>\n";
    html_file << "        body { font-family: sans-serif; display: flex; flex-direction:column; justify-content: center; align-items: center; margin: 0; background-color: #f0f2f5; }\n";
	html_file << "        p {margin: 0;}\n";
    html_file << "    </style>\n";
    html_file << "</head>\n";
    html_file << "<body>\n";


	html_file << "<h1>Traffic Simulation</h1>\n";
	html_file << "<p>Duration: " << TimeCodeVerboseString(data.total_duration) << "</p>\n";
	html_file << "<p>Road Capacity: " << data.road_capacity << "</p>\n";
	html_file << "<p>Average New Cars / Sec: " << data.new_car_rate << "</p>\n";
	html_file << "<p>Number of Crashes: " << data.num_crashes << "</p>\n";
	html_file << "<p>Total # of Cars Simulated: " << data.total_cars << "</p>\n";

	auto get_size = [&](int k) -> size_t {
		auto it = data.count_times_map.find(k);
		return (it != data.count_times_map.end()) ? it->second.size() : 0;
	};

	html_file << "The number of moments (individual seconds) in which there were 0 cars: " << get_size(0) << "</p>\n";
	html_file << "The number of moments (individual seconds) in which there was exactly 1 car: " << get_size(1) << "</p>\n";
	html_file << "The number of moments (individual seconds) in which there were only 2 cars: " << get_size(2) << "</p>\n";
	int sum = 0;
	for(int i = 0; i <= 10; i++){
		sum = sum + get_size(i);
	}
	int per = percentage(sum, data.total_duration.GetTimeCodeAsSeconds());
	html_file << "The number of moments (individual seconds) in which there were 10 cars or fewer: " << sum << " (about "  <<  per << "% of the entire simulation time.)" << "</p>\n";


	html_file << "<h4>Below is a plot of the number of cars actively traveling between 2hr 0min and 2hr 10min.</h4>\n";

    // This is the div where Plotly will render the chart
    html_file << "    <div id=\"TraffSimPlotDiv\"></div>\n";

    // Start of the JavaScript block
    html_file << "    <script>\n";

    // Convert C++ vectors to JavaScript arrays
    html_file << "        var xData = [";
    for (size_t i = 0; i < data.x_data.size(); ++i) {
        html_file << data.x_data[i];
        if (i < data.x_data.size() - 1) {
            html_file << ", ";
        }
    }
    html_file << "];\n";

    html_file << "        var yData = [";
    for (size_t i = 0; i < data.y_data.size(); ++i) {
        html_file << data.y_data[i];
        if (i < data.y_data.size() - 1) {
            html_file << ", ";
        }
    }
    html_file << "];\n";

    // Define the data trace for Plotly
    html_file << "        var trace1 = {\n";
    html_file << "            x: xData,\n";
    html_file << "            y: yData,\n";
    html_file << "            mode: 'lines',\n"; // Can be 'lines', 'markers', or 'lines+markers'
    html_file << "            type: 'scattergl',\n";
    html_file << "            marker: { size: 8, color: 'blue' }\n";
    html_file << "        };\n";

    // Define the layout for the plot
    html_file << "        var layout = {\n";
    html_file << "            title: 'Cars Traveling Over Time',\n";
    html_file << "            xaxis: { title: 'Time (seconds)' },\n";
    html_file << "            yaxis: { title: 'Number of Cars' },\n";
    html_file << "            hovermode: 'closest'\n"; // Enable tooltips on hover
    html_file << "        };\n";

    // Combine data and layout
    html_file << "        var data = [trace1];\n";

    // Render the plot 
    html_file << "        Plotly.newPlot('TraffSimPlotDiv', data, layout, {responsive: true});\n";

    html_file << "    </script>\n";
    // End of the HTML boilerplate
    html_file << "</body>\n";
    html_file << "</html>\n";

    html_file.close();
    std::cout << "HTML file generated successfully: " << filename << std::endl;
}



void generateTerminalOutput(const results& data) {
	std::cout << "\tduration: " << TimeCodeVerboseString(data.total_duration)
		<< "\n\troad capacity: " << data.road_capacity
		<< "\n\tavg new cars / second: " << data.new_car_rate
		<< "\n\tnumber of crashes: " << data.num_crashes
		<< "\n\ttotal number of cars simulated: " << data.total_cars << std::endl;

	auto get_size = [&](int k) -> size_t {
		auto it = data.count_times_map.find(k);
		return (it != data.count_times_map.end()) ? it->second.size() : 0;
	};

	std::cout << "\tThe number of moments (individual seconds) in which there were 0 cars: " << get_size(0) << std::endl;
	std::cout << "\tThe number of moments (individual seconds) in which there was exactly 1 car: " << get_size(1) << std::endl;
	std::cout << "\tThe number of moments (individual seconds) in which there were only 2 cars: " << get_size(2) << std::endl;
	int sum = 0;
	for(int i = 0; i <= 10; i++){
		sum = sum + get_size(i);
	}
	int per = percentage(sum, data.total_duration.GetTimeCodeAsSeconds());
	std::cout << "\tThe number of moments (individual seconds) in which there were 10 cars or fewer: " << sum << " (about "  <<  per << "% of the entire simulation time.)" << std::endl;
}




bool is_int(std::string a){
	for(size_t i = 0; i < a.size(); i++){
		if(!isdigit(a.at(i))){
			return false;
		}
	}
	return true;
}

unsigned long long int parse_args(int argc, char* argv[]){
	unsigned long long int duration = 0;
	if(argc != 2){
		std::cout << "Usage: traffic-simulation <duration_in_seconds>" << std::endl;
		std::exit(1);
	}

	std::string duration_str = argv[1];
	if(is_int(duration_str)){
		duration = std::stoull(duration_str); // stoull = string to unsigned long long (int)
		if(duration <= 0){
			std::cout << "Error: duration must be positive." << std::endl;
			std::exit(1);
		}
		
	} else {
		std::cout << "Error: duration must be an integer." << std::endl;
		std::exit(1);
	}

	return duration;
}


int main(int argc, char* argv[]) {
	const unsigned long long int DURATION = parse_args(argc, argv);

	std::cout << "--- Traffic Simulator ---" << std::endl;
	TimeCode dur = TimeCode(0, 0, DURATION);

	const int CAP = 200; // maximum capacity of the road
	const double new_car_rate = 4; // average new cars per second
	const double p_car_exits = 0.298; // probability a car departs when road is empty

	std::cout << "Computing crash probabilities from database..." << std::endl;
	// Build the crash probability table
	const std::unordered_map<std::string, double> crash_prob_map = build_crash_prob_map();
	for(const auto& pair : crash_prob_map){
		std::cout << "Day: " << pair.first << ", probability of a crash: " << pair.second << std::endl;
	}

	std::cout << "Simulating traffic..." << std::endl;
	std::vector<data_point_pair> data; // to store the data points
	data.reserve(DURATION);
	int num_cars = 0; // number of cars (intially 0)
	int crash_count = 0;
	int crash_timer = 0; // cool-off period of a crash

	// accumulate cars in unsigned long long, make BigInteger at the end
	unsigned long long total_num_cars_accum = 0;
	int last_progress = -1;

	for(unsigned long long sec = 0; sec < DURATION; ++sec){
		TimeCode t(0, 0, sec);

		// only update when percent changes
		int progress = percentage(sec, DURATION);
		if(progress != last_progress){
			last_progress = progress;
			std::cout << "\r" << progress << "%" << std::flush;
		}

		// removed ALL(); test call

		// --- New Cars Show Up (maybe) ---
		int num_new_cars = poisson(new_car_rate);
		if(num_cars >= CAP){
			num_new_cars = 0; // road is full, no new cars
		}
		total_num_cars_accum += num_new_cars;

		// --- Existing Cars Might Leave ---
		// Strong assumptions of model here!
		// if the road is full, the cars cannot move and therefore cannot exit
		// As the road becomes more congested, less cars can exit so we decrease the probability of an exit
		// Overall the chances are something like 30% chance of exit for each car present
		// So about 1/3 of cars leave typically.
		double cur_p_car_exits = p_car_exits * (1.0 - double(num_cars) / CAP); // a simple linear approximation
		int num_cars_exiting = binomial(num_cars, cur_p_car_exits);
		// no cars exit if there was a crash recently
		if(crash_timer > 0){
			num_cars_exiting = 0;
		}
		crash_timer--;

		// check for a crash once / day
		if(sec % DURATION_1DAY == 0){
			double prob_of_crash_today = crash_prob_map.at(day_of_week_name(day_of_week_code(t)));
			bool crash = crash_occurs(prob_of_crash_today);
			if(crash){
				///std::cout << "CRASH! No cars exit for a while now" << std::endl;
				crash_count++;
				crash_timer += DURATION_10MIN;
			}
		}

		// --- Compute Number of Cars Now ---
		int cand_num_cars = num_cars + num_new_cars - num_cars_exiting;
		if(cand_num_cars < 0){
			num_cars = 0; // road is empty
		} else {
			num_cars = cand_num_cars;
		}

		// --- Track Data for Plotting Later ---
		data.push_back({t, num_cars});
	}
	std::cout << std::endl;


	// Find the times at which certain amount of cars are present
	std::cout << "Computing sample statistics..." << std::endl;
	std::unordered_map<int, std::vector<TimeCode>> count_times;
	last_progress = -1;
	for(size_t i = 0; i < data.size(); i++){
		int progress = percentage(i, data.size());
		if(progress != last_progress){
			last_progress = progress;
			std::cout << "\r" << progress << "%" << std::flush;
		}
		// in-place push_back to avoid copying vector
		count_times[data[i].num_cars].push_back(data[i].t);
	}
	std::cout << "\n---Simulation Finished---" << std::endl;



	results res;

	res.total_duration = dur;
	res.road_capacity = CAP;
	res.new_car_rate = new_car_rate;
	res.num_crashes = crash_count;

	BigInteger total_num_cars(std::to_string(total_num_cars_accum));
	res.total_cars = total_num_cars.ToString();
	res.count_times_map = std::move(count_times);

	std::vector<int> x_data;
	std::vector<int> y_data;
	size_t start = 7200;
	size_t end = (data.size() < start + DURATION_10MIN) ? data.size() : (start + DURATION_10MIN);
	if(data.size() > start){
		for(size_t i = start; i < end; i++){
			x_data.push_back(data[i].t.GetTimeCodeAsSeconds());
			y_data.push_back(data[i].num_cars);
		}
	}
	res.x_data = x_data;
	res.y_data = y_data;
	res.plotted_duration = TimeCode(0, 0, DURATION_10MIN);
	res.plotted_duration.WasteTimeAndBeSlow();


	generateTerminalOutput(res);
	generateHTMLoutput(res);


	return 0;
}