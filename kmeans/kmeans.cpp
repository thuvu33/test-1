/*! k-means algorithm implementation based on [article](https://www.codeproject.com/Articles/5256294/Classifying-Data-Using-Artificial-Intelligence-K-M) */
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <cassert>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

using std::count;
using std::string,
	std::to_string,
	std::getline;
using std::vector,
	std::size;
using std::pair,
	std::make_pair;
using std::ifstream;
using std::istringstream;
using std::invalid_argument,
	std::runtime_error;
using std::pow;
using std::cout;
using boost::algorithm::join;
using boost::adaptors::transformed;


using table_type = vector<pair<vector<double>, int>>;  //!< N columns of features as doubles and label as integer
using cluster_type = pair<size_t, vector<size_t>>;  //!< centroid idx

table_type load_data_from_file(string const & fname);
void kmeans(table_type const & ds, size_t k);


double distance(vector<double> const & u, vector<double> const & v)
{
	assert(size(u) == size(v));

	double result = 0;
	for (size_t i = 0; i < size(u); ++i)
		result += pow(u[i] - v[i], 2);

	return result;
}

// vrati index zaznam z najvacsou vzdialenostou od centroid
size_t euclidian_step(table_type const & ds, size_t centroid)
{
	size_t max_idx = 0;
	double max_distance = 0.0;
	for (size_t i = 0; i < size(ds); ++i)
	{
		double dist = distance(ds[centroid].first, ds[i].first); 
		if (dist > max_distance)
		{
			max_idx = i;
			max_distance = dist;
		}
	}
	return max_idx;
}

// get list of initial clusters
vector<cluster_type> lloyd_step(table_type const & ds, 
	vector<size_t> const & centroids)
{
	vector<cluster_type> clusters(size(centroids));
	for (size_t i = 0; i < size(centroids); ++i)
		clusters[i].first = centroids[i];

	// find cluster members
	for (size_t i = 0; i < size(ds); ++i)
	{
		size_t cluster_idx = 0;

		double min_dist = 0;
		for (size_t j = 0; j < size(clusters); ++j)
		{
			double dist = distance(ds[i].first, ds[centroids[j]].first);
			if (dist < min_dist)
			{
				cluster_idx = j;
				min_dist = dist;
			}
		}

		clusters[cluster_idx].second.push_back(i);
	}

	return clusters;
}

// \returns list of centroids and list of clusters
pair<vector<size_t>, vector<cluster_type>> kmeans_plus_plus(
	table_type const & ds, size_t k)
{
	assert(k > 2);  // TODO: implement for k <= 2

	vector<size_t> centroids;
	centroids.reserve(k);

	// insert randomly selected first centroid
	centroids.push_back(0);

	// insert second centroid (most far record index)
	centroids.push_back(euclidian_step(ds, centroids[0]));

	vector<cluster_type> clusters;

	// rest of centroids
	for (size_t i = k - 2; i > 0; --i)
	{
		clusters = lloyd_step(ds, centroids);

		/* for each cluster compute item with the largest distance 
		to its centroid, which will becomes new cluster centroid */
		double max_distance = 0;
		size_t max_cluster_idx, max_item_idx;
		for (size_t cluster_idx = 0; cluster_idx < size(clusters); ++cluster_idx)
		{
			size_t centroid_idx = clusters[cluster_idx].first;
			
			vector<size_t> const & cluster_items = clusters[cluster_idx].second;
			for (size_t item_idx = 0; item_idx < size(cluster_items); ++item_idx)
			{
				double dist = distance(ds[centroid_idx].first, 
					ds[cluster_items[item_idx]].first);
				
				if (dist > max_distance)
				{
					max_cluster_idx = cluster_idx;
					max_item_idx = item_idx;
					max_distance = dist; 
				}
			}
		}

		centroids.push_back(clusters[max_cluster_idx].second[max_item_idx]);
	}

	return make_pair(centroids, clusters);
}

void kmeans(table_type const & ds, size_t k)
{

}


int main(int argc, char * argv[])
{
	string const dataset_file = argc > 1 ? argv[1] : "dataset.csv";
	size_t const k = argc > 2 ? atoi(argv[2]) : 5;

	table_type ds = load_data_from_file(dataset_file);
	cout << "records: " << size(ds) << "\n";

	for (pair<vector<double>, int> const & row : ds)
		cout << join(row.first|transformed(static_cast<string (*)(double)>(to_string)), ", ")
			<< " -> " << row.second << "\n";

//	// testing kmeans_plus_plus
//	pair<vector<size_t>, vector<cluster_type>> centroids_and_clusters =
//		kmeans_plus_plus(ds, k);

//	cout << "centroids:\n";
//	vector<size_t> const & centroids = centroids_and_clusters.first;
//	for (int centroid : centroids)
//		cout << "  " << centroid << "\n";

	cout << "done!\n";
	return 0;
}

table_type load_data_from_file(string const & fname)
{
	ifstream fin{fname};
	if (!fin.is_open())
		throw invalid_argument{"unable to open \"" + fname + "\" file"};

	table_type result;

	string line;
	getline(fin, line);

	size_t const columns = count(begin(line), end(line), ',');
	if (columns < 1)  // not enough columns
		throw runtime_error{"unable to parse table record (\"" + line + "\" at least two columns expected"};  // wrong number of columns

	vector<double> row;
	row.resize(columns);

	do
	{
		istringstream in{line};
		char sep;
		for (size_t i = 0; i < columns; ++i)
			in >> row[i] >> sep;

		int label;
		in >> label;

		if (in.bad())
			throw runtime_error{"unable to parse table record (\"" + line + "\""};

		result.push_back(make_pair(row, label));
	}
	while(getline(fin, line));

	return result;
}
