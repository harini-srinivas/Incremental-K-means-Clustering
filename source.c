
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>


#define MIN_DIST_CENTROID_NOT_FOUND -1
#define NUM_ITEMS_FOR_THRESHOLD 250
#define MIN(a, b) ( (  ( a < b) ? a : b ) )

/****Function declarations****/
void calculateThreshold(float **thresholdPtrPtr, float *thresholdP, int *newItems, int newItemsCount);
void read_data(int **newItems, int *newItemCount, float **existingCentroids, int *existingCentroidsCount, int **existingGroups, int *existingGroupsCount, float **thresholdPtrPtr, float *thresholdP);
void print_results(int *groups, int itemsCount, float *centroids, int centroidsCount, float threshold);

/****
Reads files ...
new_items.txt -- contains new data items (one entry per line).
centroids_in.txt -- centroid float values (one per line).
groups_in.txt -- for each data item contains the cluster ID for that item.
threshold.txt -- contains one float value of initial threshold.
****/
void read_data(
	int **newItems, int *newItemCount,
	float **existingCentroids, int *existingCentroidsCount,
	int **existingGroups, int *existingGroupsCount,
	float **thresholdPtrPtr, float *thresholdP){
	FILE * fp;
	char buf[64];
	int scanfRetValue;
	float thresholdFileValue;

	*newItemCount = 0;
	fp = fopen("500000.txt", "r");
	if (fp == NULL) perror("Error opening file -- new_items.txt !");
	while (fgets(buf, 64, fp) != NULL) {
		*newItemCount += 1;
		*newItems = (int*)realloc(*newItems, (*newItemCount)*sizeof(int));
		(*newItems)[*newItemCount - 1] = atoi(buf);
	}
	fclose(fp);

	*existingCentroidsCount = 0;
	fp = fopen("centroids_in.txt", "r");
	while (fgets(buf, 64, fp) != NULL){
		*existingCentroidsCount += 1;
		*existingCentroids = (float*)realloc(*existingCentroids, (*existingCentroidsCount)*sizeof(float));
		(*existingCentroids)[*existingCentroidsCount - 1] = atof(buf);
	}
	fclose(fp);

	*existingGroupsCount = 0;
	fp = fopen("groups_in.txt", "r");
	while (fgets(buf, 64, fp) != NULL){
		*existingGroupsCount += 1;
		*existingGroups = (int*)realloc(*existingGroups, (*existingGroupsCount)*sizeof(int));
		(*existingGroups)[*existingGroupsCount - 1] = atoi(buf);
	}
	fclose(fp);

	fp = fopen("threshold.txt", "r");
	//If file "threshold.txt" does not exist
	if (!fp) {
		calculateThreshold(thresholdPtrPtr, thresholdP, *newItems, *newItemCount);
		return;
	}

	//read line into buffer
	if (fgets(buf, 64, fp) != NULL) {
		calculateThreshold(thresholdPtrPtr, thresholdP, *newItems, *newItemCount);
		return;
	}

	//Convert the string 'buf' to a float
	scanfRetValue = sscanf(buf, "%f", &thresholdFileValue);

	//If the file 'threshold.txt' does not contain a valid float number,
	//then call the calculate threshold function.
	if ((scanfRetValue == EOF) || (scanfRetValue == 0)) {
		calculateThreshold(thresholdPtrPtr, thresholdP, *newItems, *newItemCount);
		return;
	}

	//Everthing ok in file 'threshold.txt' so store 
	*thresholdPtrPtr = thresholdP;
	*thresholdP = thresholdFileValue;
}

void print_results(int *groups, int itemsCount, float *centroids, int centroidsCount, float threshold){ 
	FILE *fp;
	int i;

	fp = fopen("groups_out.txt", "w");
	for (i = 0; i < itemsCount; ++i)
		fprintf(fp, "%d\n", groups[i]);
	fclose(fp);

	fp = fopen("centroids_out.txt", "w");
	for (i = 0; i < centroidsCount; ++i)
		fprintf(fp, "%0.3f\n", centroids[i]);
	fclose(fp);

	fp = fopen("threshold.txt", "w");
	fprintf(fp, "%0.3f\n", threshold); 
	fclose(fp);

	printf("Data written to files groups_out.txt, centroids_out.txt, threshold.txt!\n");
}

void calculateThreshold(float **thresholdPtrPtr, float *thresholdP, int *newItems, int newItemsCount) {
	/***
	Compute the distances between each pair of new items and take the mean value.
	Assume distance is ABS(item1 - item2) raised to power of 2
	For example,
	if the items are (5, 7 and 10) the distances are
	...5-7 = 4
	...5-10 = 25
	...7-10 = 9
	mean value = 38/3 = 12.667
	***/
	int i, j, total_dist, loopIndex, temp;

	
	if (*thresholdPtrPtr == NULL) { 
		loopIndex = MIN(NUM_ITEMS_FOR_THRESHOLD, newItemsCount);
		for (i = 0; i < loopIndex; i++)
			for (j = i + 1; j < loopIndex; j++) {
				temp = abs(newItems[i] - newItems[j]);
				total_dist += temp * temp;
			}

		*thresholdPtrPtr = thresholdP;
		*thresholdP = total_dist / ((loopIndex * (loopIndex - 1)) / 2);


	}
}

void calculateClusterNumItemsArray(int **clusterNumItemsPtrPtr, int *existingGroups, int existingGroupsCount, int existingCentroidCount) {

	if (existingCentroidCount == 0)
		return;
	*clusterNumItemsPtrPtr = (int *)malloc(existingCentroidCount * sizeof(int));
	int *cluster_num_items = *clusterNumItemsPtrPtr;
	int i;
	
	for (i = 1; i < existingGroupsCount; i++)
	{
		cluster_num_items[existingGroups[i]]++;
	}

	/****
	Use the existingGroups array to calculate the cluster_num_items array
	For example, if existingGroups = [0 3 5 2 4 3 4 0 4 5 4 3 3 3 1 1 1 1] then
	clusterNumItems will be [2 4 1 5 3 2] which corresponds to
	2 cluster 0 items
	4 cluster 1 items
	1 cluster 2 items
	5 cluster 3 items
	3 cluster 4 items
	2 cluster 5 items
	****/
}

int getMinimumDistanceCentroidIndex(int newItem, float *existingCentroids, int existingCentroidsCount, float threshold) {

	int i, temp, minimum, minimum_index;
	int *dist;
	if (existingCentroidsCount == 0)
		return MIN_DIST_CENTROID_NOT_FOUND;

	dist = (int *)malloc(existingCentroidsCount * sizeof(int));
	for (i = 0; i < existingCentroidsCount; i++)
	{
		temp = abs(existingCentroids[i] - newItem);
		dist[i] = temp * temp;
	}

	minimum = dist[0];
	minimum_index = 0;

	for (i = 1; i < existingCentroidsCount; i++)
	{
		if (minimum > dist[i])
		{
			minimum = dist[i];
			minimum_index = i;
		}
	}

	if (minimum > threshold)
		return MIN_DIST_CENTROID_NOT_FOUND;
	else
		return minimum_index;


	/****
	if centroid list is empty
	return MIN_DIST_CENTROID_NOT_FOUND


	for each centroid
	calculate distance of new item from centroid.
	end for
	With all distances, find minimum_distance and index_of_centroid
	if minimum_distance > Threshhold

	return MIN_DIST_CENTROID_NOT_FOUND
	else
	return index_of_centroid
	****/
}

void centroid_calculation(float *centroids, int index_of_centroid, int cluster_num_items, int new_item)
{
	centroids[index_of_centroid] = ((centroids[index_of_centroid] * cluster_num_items) + new_item) / (cluster_num_items + 1);
}

void addNewCluster(
	int newItem,
	int *existingGroups, int *existingGroupsCountPtr,
	float *existingCentroids, int *existingCentroidsCountPtr, int *clusterNumItems) { 

	/******
	Add newItem to existingCentroids
	Add new cluster to groups
	Increment existingCentroidsCount
	Increment existingGroupsCount

	[existingCentroidsCount]
	existingGroups[existingGroupsCount] =
	******/
	existingCentroids[*existingCentroidsCountPtr] = newItem;
	existingGroups[*existingGroupsCountPtr] = *existingCentroidsCountPtr;
	clusterNumItems[*existingCentroidsCountPtr] = 1;
	*existingCentroidsCountPtr += 1;
	*existingGroupsCountPtr += 1;

}

void addToExistingCluster(int minDistClusterIdx, int newItem, int *existingGroups, int *existingGroupsCountPtr, int *clusterNumItems, float *existingCentroids) { 

	centroid_calculation(existingCentroids, minDistClusterIdx, clusterNumItems[minDistClusterIdx], newItem);  
	existingGroups[*existingGroupsCountPtr] = minDistClusterIdx;  
	clusterNumItems[minDistClusterIdx] += 1;  
	*existingGroupsCountPtr += 1;
	/****
	Update existingGroups with min dist index
	calculate new centroid value by using (old centroid value and clusterNumItems and newItem value).
	Update existingCentroids value with the calculated centroid value.
	Also, update the cluster_num_items array 
	****/
}


int main(){


	int i;

	//This array (INPUT) will hold all the new items that are input to the program.
	int *newItems = NULL;

	//This will contain the count of new items 
	int newItemsCount;

	//This array (INPUT) will hold the cluster ID (0...k) for each data item.
	//For example, if the 5th data item belongs to 7th cluster and 10th data item belongs to 3rd cluster then
	//existingGroups[4] = 6 and existingGroups[9] = 2
	int *existingGroups = NULL;

	//This will contain the size of the existingGroups array
	int existingGroupsCount;

	//This array (INPUT) will contain the centroids identified thus far.
	//This could be input to the program from the centroids_in.txt file.
	float *existingCentroids = NULL;

	//This contains the number of centroids.
	int existingCentroidsCount;

	//This array (CALCULATED) will contain number of items for each cluster.
	//For example, if the 4th cluster contains 15 items and the 20th cluster contains 3 items
	//  clusterNumItems[3] = 15, clusterNumItems[19] = 3
	int *clusterNumItems = NULL;

	//This will contain the Threshold value
	//This will either be read from a file or calculated with new items.
	float threshold;
	float *thresholdPtr = NULL;

	FILE *fp1, *fp2; 
	char ch; 

	/* read data from files on cpu */
	//Allocate new_items, existingCentroids, existingGroups and their counts
	read_data(&newItems, &newItemsCount,
		&existingCentroids, &existingCentroidsCount,
		&existingGroups, &existingGroupsCount, &thresholdPtr, &threshold);

	if (newItemsCount == 0) {
		printf("No new items.\n");

		//TODO: Copy groups_in.txt to groups_out.txt
		fp1 = fopen("groups_in.txt", "r");		
		fp2 = fopen("groups_out.txt", "w");
		while ((ch = fgetc(fp1)) != EOF) {
			putc(ch, fp2);
		}
		fclose(fp1);
		fclose(fp2);

		//TODO: Copy centroids_in.txt to centroids_out.txt
		fp1 = fopen("centroids_in.txt", "r");		
		fp2 = fopen("centroids_out.txt", "w");
		while ((ch = fgetc(fp1)) != EOF) {
			putc(ch, fp2);
		}
		fclose(fp1);
		fclose(fp2);

	}

	//STEPS::
	//  Read input
	//    ...new data items from file new_items.txt
	//    ...existing centroids from file centroids_in.txt
	//    ...existing groups from file groups_in.txt
	//    ...threshold value from file threshold.txt

	//  Calculate Threshold value using new items and existing threshold if any.
	calculateThreshold(&thresholdPtr, &threshold, newItems, newItemsCount);

	//  Calculate ClusterNumItems array
	calculateClusterNumItemsArray(&clusterNumItems, existingGroups, existingGroupsCount, existingCentroidsCount); 

	//  FOR LOOP eachNewItem in newItems[1...N]
	//    centroid_found = getMininumDistance()
	//    if centroid_found
	//       addToExistingCluster(...)
	//    else
	//       addToNewCluster(...)
	//    end if
	//  END FOR
	//
	//  print output
	//      existingGroups array in file groups_out.txt
	//      existingCentroids array in file centroids_out.txt
	//      threshold value in file threshold.txt

	//Increase groups storage by newItemsCount + existingGroupsCount
	existingGroups = (int*)realloc(existingGroups, (newItemsCount + existingGroupsCount) * sizeof(int));
	//Increase centroids storage by newItemsCount + existingCentroidsCount
	existingCentroids = (float*)realloc(existingCentroids, (newItemsCount + existingCentroidsCount) * sizeof(float));
	//Increase clusterNumItems storage
	clusterNumItems = (int*)realloc(clusterNumItems, (newItemsCount + existingCentroidsCount) * sizeof(int));

	for (i = 0; i < newItemsCount; i++) {
		int newItem = newItems[i];
		int minDistCentroidIndex =
			getMinimumDistanceCentroidIndex(
			newItem, existingCentroids, existingCentroidsCount, threshold);
		if (minDistCentroidIndex == MIN_DIST_CENTROID_NOT_FOUND) {
			addNewCluster(newItem, existingGroups, &existingGroupsCount, existingCentroids, &existingCentroidsCount, clusterNumItems); 
		}
		else {
			addToExistingCluster(minDistCentroidIndex, newItem, existingGroups, &existingGroupsCount, clusterNumItems, existingCentroids); 
		}

	}

	print_results(existingGroups, existingGroupsCount, existingCentroids, existingCentroidsCount, threshold); 

	
	//Calculate Threshold Value
	//KMEANS_CPU
	return 0;
}




