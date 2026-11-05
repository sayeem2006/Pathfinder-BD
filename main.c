#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_STOPS 30
#define MAX_EDGES 100
#define NAME_LEN 64
#define INF 1000000000

typedef struct {
    char name[NAME_LEN];
} Stop;

typedef struct {
    int from;
    int to;
    int fare;
    char bus[NAME_LEN];
} Edge;

static Stop stops[MAX_STOPS];
static Edge edges[MAX_EDGES];
static int stop_count = 0, edge_count = 0;

static int same_text(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static int add_stop(const char *name) {
    int i;
    for (i = 0; i < stop_count; i++)
        if (same_text(stops[i].name, name)) return i;
    if (stop_count >= MAX_STOPS) return -1;
    snprintf(stops[stop_count].name, NAME_LEN, "%s", name);
    return stop_count++;
}

static void add_connection(const char *from, const char *to, int fare, const char *bus) {
    int a = add_stop(from), b = add_stop(to);
    if (a < 0 || b < 0 || edge_count >= MAX_EDGES) return;

    edges[edge_count].from = a;
    edges[edge_count].to = b;
    edges[edge_count].fare = fare;
    snprintf(edges[edge_count].bus, NAME_LEN, "%s", bus);
    edge_count++;

    /* Buses can travel in either direction on this demo network. */
    edges[edge_count].from = b;
    edges[edge_count].to = a;
    edges[edge_count].fare = fare;
    snprintf(edges[edge_count].bus, NAME_LEN, "%s", bus);
    edge_count++;
}

static void load_demo_data(void) {
    /* IMPORTANT: These are illustrative demo routes and fares, not verified live data. */
    add_connection("Mirpur 10", "Mirpur 11", 10, "Mirpur Link");
    add_connection("Mirpur 11", "Pallabi", 10, "Mirpur Link");
    add_connection("Mirpur 10", "Kazipara", 10, "City Express");
    add_connection("Kazipara", "Shewrapara", 10, "City Express");
    add_connection("Shewrapara", "Agargaon", 10, "City Express");
    add_connection("Agargaon", "Farmgate", 15, "City Express");
    add_connection("Farmgate", "Karwan Bazar", 10, "City Express");
    add_connection("Karwan Bazar", "Shahbag", 10, "City Express");
    add_connection("Shahbag", "Science Lab", 10, "City Express");
    add_connection("Science Lab", "Dhanmondi", 10, "City Express");

    add_connection("Mirpur 10", "Shyamoli", 20, "North-South Bus");
    add_connection("Shyamoli", "Asad Gate", 10, "North-South Bus");
    add_connection("Asad Gate", "Dhanmondi", 10, "North-South Bus");
    add_connection("Asad Gate", "Farmgate", 15, "North-South Bus");

    add_connection("Pallabi", "Kalshi", 10, "Eastern Route");
    add_connection("Kalshi", "ECB Chattar", 10, "Eastern Route");
    add_connection("ECB Chattar", "Banani", 15, "Eastern Route");
    add_connection("Banani", "Mohakhali", 10, "Eastern Route");
    add_connection("Mohakhali", "Farmgate", 15, "Eastern Route");
    add_connection("Farmgate", "Dhanmondi", 20, "Campus Shuttle");
}

static int find_stop(const char *name) {
    int i;
    for (i = 0; i < stop_count; i++)
        if (same_text(stops[i].name, name)) return i;
    return -1;
}

static void show_stops(void) {
    int i;
    printf("\nAvailable stops:\n");
    for (i = 0; i < stop_count; i++)
        printf("  %2d. %s\n", i + 1, stops[i].name);
}

static void show_connections(void) {
    int i;
    printf("\nDemo bus connections (illustrative fares):\n");
    for (i = 0; i < edge_count; i++) {
        /* Print each undirected connection only once. */
        if (edges[i].from < edges[i].to) {
            printf("  %-18s -> %-18s | %-18s | Tk %d\n",
                   stops[edges[i].from].name, stops[edges[i].to].name,
                   edges[i].bus, edges[i].fare);
        }
    }
}

static void print_route(int source, int destination, int parent_stop[],
                        int parent_edge[], int total_fare, const char *title) {
    int path[MAX_STOPS], path_edges[MAX_STOPS], count = 0, i;
    int current = destination, transfers = 0;
    char previous_bus[NAME_LEN] = "";

    while (current != -1 && count < MAX_STOPS) {
        path[count] = current;
        path_edges[count] = parent_edge[current];
        count++;
        if (current == source) break;
        current = parent_stop[current];
    }

    if (count == 0 || path[count - 1] != source) {
        printf("\nNo route found.\n");
        return;
    }

    printf("\n%s\n", title);
    printf("Route: ");
    for (i = count - 1; i >= 0; i--) {
        printf("%s", stops[path[i]].name);
        if (i > 0) printf(" -> ");
    }
    printf("\n");

    printf("Journey details:\n");
    for (i = count - 2; i >= 0; i--) {
        int e = path_edges[i];
        if (e >= 0) {
            if (previous_bus[0] && !same_text(previous_bus, edges[e].bus))
                transfers++;
            snprintf(previous_bus, NAME_LEN, "%s", edges[e].bus);
            printf("  %s to %s by %s (Tk %d)\n",
                   stops[edges[e].from].name, stops[edges[e].to].name,
                   edges[e].bus, edges[e].fare);
        }
    }
    printf("Estimated total fare: Tk %d\n", total_fare);
    printf("Bus changes: %d\n", transfers);
    printf("Note: Demo estimates only; actual routes, fares and availability may differ.\n");
}

static void cheapest_route(int source, int destination) {
    int dist[MAX_STOPS], used[MAX_STOPS] = {0};
    int parent[MAX_STOPS], parent_edge[MAX_STOPS];
    int i, step, u = -1;

    for (i = 0; i < stop_count; i++) {
        dist[i] = INF;
        parent[i] = -1;
        parent_edge[i] = -1;
    }
    dist[source] = 0;

    for (step = 0; step < stop_count; step++) {
        int best = INF;
        u = -1;
        for (i = 0; i < stop_count; i++) {
            if (!used[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }
        if (u == -1 || u == destination) break;
        used[u] = 1;

        for (i = 0; i < edge_count; i++) {
            if (edges[i].from == u) {
                int v = edges[i].to;
                if (!used[v] && dist[u] + edges[i].fare < dist[v]) {
                    dist[v] = dist[u] + edges[i].fare;
                    parent[v] = u;
                    parent_edge[v] = i;
                }
            }
        }
    }

    if (dist[destination] == INF) {
        printf("\nNo route found between those stops.\n");
        return;
    }
    print_route(source, destination, parent, parent_edge, dist[destination],
                "CHEAPEST ROUTE");
}

static void fewest_stops_route(int source, int destination) {
    int distance[MAX_STOPS], parent[MAX_STOPS], parent_edge[MAX_STOPS];
    int queue[MAX_STOPS], front = 0, rear = 0, i;

    for (i = 0; i < stop_count; i++) {
        distance[i] = -1;
        parent[i] = -1;
        parent_edge[i] = -1;
    }

    distance[source] = 0;
    queue[rear++] = source;

    while (front < rear) {
        int u = queue[front++];
        if (u == destination) break;

        for (i = 0; i < edge_count; i++) {
            if (edges[i].from == u && distance[edges[i].to] == -1) {
                int v = edges[i].to;
                distance[v] = distance[u] + 1;
                parent[v] = u;
                parent_edge[v] = i;
                queue[rear++] = v;
            }
        }
    }

    if (distance[destination] == -1) {
        printf("\nNo route found between those stops.\n");
        return;
    }

    /* Calculate fare along the BFS-selected route. */
    {
        int total_fare = 0, current = destination;
        while (current != source) {
            int e = parent_edge[current];
            if (e < 0) break;
            total_fare += edges[e].fare;
            current = parent[current];
        }
        print_route(source, destination, parent, parent_edge, total_fare,
                    "ROUTE WITH FEWEST BUS SEGMENTS");
        printf("Number of bus segments: %d\n", distance[destination]);
    }
}

static void read_line(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

static void find_journey(void) {
    char from[NAME_LEN], to[NAME_LEN];
    int source, destination, choice;

    show_stops();
    printf("\nEnter starting stop exactly as shown: ");
    read_line(from, sizeof(from));
    printf("Enter destination exactly as shown: ");
    read_line(to, sizeof(to));

    source = find_stop(from);
    destination = find_stop(to);

    if (source < 0 || destination < 0) {
        printf("\nSorry, one or both stop names were not found. Check the available stops list.\n");
        return;
    }
    if (source == destination) {
        printf("\nYou are already at %s. No bus journey is needed.\n", stops[source].name);
        return;
    }

    printf("\nChoose what matters most:\n");
    printf("  1. Cheapest route\n");
    printf("  2. Route with fewest bus segments\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        printf("Invalid choice.\n");
        return;
    }
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }

    if (choice == 1) cheapest_route(source, destination);
    else if (choice == 2) fewest_stops_route(source, destination);
    else printf("Invalid choice. Please select 1 or 2.\n");
}

int main(void) {
    int choice;
    load_demo_data();

    printf("=====================================\n");
    printf("       PATHFINDER BD - C PROJECT\n");
    printf("  A simple public bus route finder\n");
    printf("=====================================\n");
    printf("This project uses sample stops, routes and fares for demonstration.\n");

    while (1) {
        printf("\n--------------- MENU ----------------\n");
        printf("1. Find a bus route\n");
        printf("2. Show all available stops\n");
        printf("3. Show demo bus connections and fares\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Please enter a number from 1 to 4.\n");
            continue;
        }
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
        }

        switch (choice) {
            case 1: find_journey(); break;
            case 2: show_stops(); break;
            case 3: show_connections(); break;
            case 4:
                printf("\nThank you for using Pathfinder BD. Safe travels!\n");
                return 0;
            default:
                printf("Invalid option. Please choose 1 to 4.\n");
        }
    }
}
