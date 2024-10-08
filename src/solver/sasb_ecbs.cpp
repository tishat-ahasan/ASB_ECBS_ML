
/*
 * sasb_ecbs.cpp
 *
 * Purpose: Static Agent-Specific Sub-Optimal Bounded ECBS
 *
 * An Adaptive Agent-Specific Sub-Optimal Bounding Approach for Multi-Agent Path Finding.
 *
 * Created by: Mustafizur Rahman <mustafizz996@gmail.com>
 */
#include "sasb_ecbs.h"
#include "../util/util.h"
#include <bits/stdc++.h>
#include <fstream> // Include for file operations
using namespace std;

uint64_t SASB_ECBS::timeSinceEpochMillisec()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

SASB_ECBS::SASB_ECBS(Problem *_P, float _w) : CBS(_P, false), w(_w)
{
  init();
}
SASB_ECBS::SASB_ECBS(Problem *_P, float _w, bool _ID) : CBS(_P, _ID), w(_w)
{
  init();
}

SASB_ECBS::~SASB_ECBS() {}

void SASB_ECBS::init()
{
  for (auto a : A)
    table_fmin.emplace(a->getId(), 0);
  cnt = 0;
  conflict_cnt = 0;
}

bool SASB_ECBS::solvePart(Paths &paths, Agents &block)
{
  CTNode *node;
  Constraints constraints;

  int uuid = 0;
  int key, keyF;
  std::unordered_set<int> OPEN;
  std::vector<int> FOCUL; // for FOCUL search
  float ub;

  bool status = true;
  bool updateMin = true;

  std::vector<CTNode *> table;
  std::vector<int> table_conflict;

  uint64_t total_time = 0;

  uint64_t current_time1 = timeSinceEpochMillisec();

  for (auto a : block)
  {
    (*a).m_w = w;
  }

  std::ofstream outputFile("output.txt");

  //cout << "W: " << w << "\n";

  CTNode *root = new CTNode{{}, {}, 0, nullptr, true, {}, 0};
  highLevelNode++;
  invoke(root, block);
  OPEN.insert(uuid);
  table.push_back(root);
  table_conflict.push_back(h3(root->paths));
  ++uuid;

  int maxi = INT_MIN;
  //int maxi = 0;
  uint64_t current_time3 = timeSinceEpochMillisec();

  // Conflict calculation
  for (auto a : block)
  {
    auto itr = std::find_if(A.begin(), A.end(),
                            [a](Agent *b)
                            { return a == b; });
    int d = std::distance(A.begin(), itr);
    (*a).conf = h3(a, root->paths[d], root->paths);
    maxi = max(maxi, (*a).conf);
    //cout << "Path: " << paths << "\n";

  }

  // Static weight assignment
  double offset = (w - 1) / maxi;

  double add = 0;
  int n = 1;
  
  //cout << "S.O. Bound,Conflict,Distance,A_Cost-S,A_Cost/S,A_Cost-LB,A_Cost/LB,Nearest Neighbor,Start Loc. Min. Dist.,Start Loc. Max. Dist.,Start Loc. Median Dist,Goal Loc. Min. Dist.,Goal Loc. Max. Dist.,Goal Loc. Median Dist.\n";
  
  int min_count1 = 0, sumOfCostMinimal = 0; 

  for (auto a : block)
  {
  
    (*a).m_w = 1 + ((*a).conf * offset);
    outputFile << (*a).m_w << "," << (*a).conf << "," << pathDist((*a).getGoal(), (*a).getNode()) << ",";
    int s_max = INT_MIN, s_min = INT_MAX, s_mdn = 0, s_sum = 0, tmp_a = 0;
    int g_max = INT_MIN, g_min = INT_MAX, g_mdn = 0, g_sum = 0;
    int agentNm = 1;

    sumOfCostMinimal += pathDist((*a).getNode(), (*a).getGoal());
    outputFile << sumOfCostMinimal - (*a).m_w << "," << (*a).m_w / sumOfCostMinimal << ",";
    outputFile << w - (*a).m_w << "," << (*a).m_w / w << ",";
    

    if ((*a).conf > 0){
      min_count1++;
    }

    for(auto b : block){
      int tmp_s = pathDist((*a).getNode(), (*b).getNode());
      int tmp_g = pathDist((*a).getGoal(), (*b).getGoal());
      s_sum += tmp_s;
      g_sum += tmp_g;

      
      if(s_max<tmp_s){
        s_max = tmp_s;
      }
      if(g_max<tmp_g){
        g_max = tmp_g;
      }
      if(s_min>tmp_s && tmp_s!=0){
        s_min = tmp_s;
        tmp_a = b->getId();
      }
      if(g_min>tmp_g && tmp_g!=0){
        g_min = tmp_g;
      }
      agentNm++;
    }

    double s_avg = s_sum/agentNm;
    double g_avg = g_sum/agentNm;
    outputFile << tmp_a << "," << s_min << "," << s_max << "," << s_avg << "," << g_min << "," << g_max << "," << g_avg << "\n";
    n++;
  }


  //cout << "The number of agents that have at least one conflict with each other: " << min_count1 << "\n";
  //cout << "The sum of costs of the individually cost-minimal paths of all agents: " << sumOfCostMinimal << "\n";

  uint64_t current_time4 = timeSinceEpochMillisec();

  bool CAT = std::any_of(paths.begin(), paths.end(),
                         [](Nodes p)
                         { return !p.empty(); });

  auto itrO = OPEN.begin();

  while (!OPEN.empty())
  {
    //cout << "===================" << "\n";
    for (auto a : table_conflict){
    //cout << "Test Print:" << a << "\n" ;
  }
    //maxi = INT_MIN;
    uint64_t current_time2 = timeSinceEpochMillisec();
    if (current_time2 - current_time1 >= P->getTimeLimit())
      return false;
    if (updateMin)
    {
      itrO = std::min_element(OPEN.begin(), OPEN.end(),
                              [CAT, this, &paths, &table](int a, int b)
                              {
                                CTNode *nA = table[a];
                                CTNode *nB = table[b];
                                if (CAT && nA->cost == nB->cost)
                                {
                                  return this->countCollisions(nA, paths) < this->countCollisions(nB, paths);
                                }
                                return nA->cost < nB->cost;
                              });
    }

    key = *itrO;
    node = table[key];

    ub = node->LB * w;
    // update focul
    FOCUL.clear();
    for (auto keyO : OPEN)
    {
      if ((float)table[keyO]->cost <= ub)
        FOCUL.push_back(keyO);
            }
    auto itrF = std::min_element(FOCUL.begin(), FOCUL.end(),
                                 [&table, &table_conflict](int a, int b)
                                 {
                                   int sA = table_conflict[a];
                                   int sB = table_conflict[b];
                                   if (sA == sB)
                                   {
                                     return table[a]->cost < table[b]->cost;
                                   }
                                   return sA < sB;
                                 });

    keyF = *itrF;
    node = table[keyF];

    conflict_cnt++;
    constraints = valid(node, block);
    if (constraints.empty())
      break;

    updateMin = (key == keyF);
    auto itrP = std::find(OPEN.begin(), OPEN.end(), keyF);
    OPEN.erase(itrP);

    for (auto constraint : constraints)
    {
      CTNode *newNode = new CTNode{constraint, node->paths, 0, node, true, {}, 0};
      highLevelNode++;
      // formating
      Node *g;
      Nodes p;
      for (int i = 0; i < node->paths.size(); ++i)
      {
        p = newNode->paths[i];
        if (p.empty())
          continue;
        g = *(p.end() - 1);
        while (*(p.end() - 1) == g)
          p.erase(p.end() - 1);
        p.push_back(g);
        node->paths[i] = p;
      }
      newNode->fmins = node->fmins;
      invoke(newNode, block);
      if (newNode->valid)
      {
        OPEN.insert(uuid);
        table.push_back(newNode);
        table_conflict.push_back(h3(newNode->paths));
        ++uuid;
      }
    }
    constraints.clear();

  }

  if (!OPEN.empty())
  { // success
    for (int i = 0; i < paths.size(); ++i)
    {
      if (!node->paths[i].empty())
      {
        paths[i] = node->paths[i];
      }
    }
    status = status && true;
  }
  else
  {
    status = false;
  }

  lowlevelnode = cnt;

 

  return status;
}

void SASB_ECBS::invoke(CTNode *node, Agents &block)
{
  int d;
  // calc path
  if (node->c.empty())
  { // initial
    Paths paths;
    for (int i = 0; i < A.size(); ++i)
    {
      paths.push_back({});
      node->fmins.push_back(0);
    }
    for (auto a : block)
    {
      auto itr = std::find_if(A.begin(), A.end(),
                              [a](Agent *b)
                              { return a == b; });
      int d = std::distance(A.begin(), itr);
      //cout << "Dist: " << (A.end() - itr) << "\n";
      paths[d] = AstarSearch(a, node);
      node->fmins[d] = table_fmin.at(a->getId());
    }
    node->paths = paths;
    node->LB = 0;
  }
  else
  {
    Agent *a;
    // error check
    if (node->paths.size() != A.size())
    {
      std::cout << "error@ECBS@invoke, "
                << "path size is not equal to the size of agents"
                << "\n";
      std::exit(1);
    }

    for (auto c : node->c)
    {
      a = c->a;
      auto itr = std::find_if(A.begin(), A.end(),
                              [a](Agent *b)
                              { return a == b; });
      if (itr == A.end())
      {
        std::cout << "error@ECBS@invoke, cannot find1 agent"
                  << "\n";
        std::exit(1);
      }
      d = std::distance(A.begin(), itr);
      node->paths[d].clear();
    }

    for (auto c : node->c)
    {
      a = c->a;
      auto itr = std::find_if(A.begin(), A.end(),
                              [a](Agent *b)
                              { return a == b; });
      if (itr == A.end())
      {
        std::cout << "error@ECBS@invoke, cannot find2 agent"
                  << "\n";
        std::exit(1);
      }
      d = std::distance(A.begin(), itr);
      node->paths[d] = AstarSearch(a, node);
      node->fmins[d] = table_fmin.at(a->getId());
    }
  }
  for (auto i : node->fmins)
    node->LB += i;

  if (!node->valid)
    return;
  calcCost(node, block);
  formalizePathAgents(node, block);
}

int SASB_ECBS::h3(Agent *a, Nodes &p1, Paths &paths)
{
  if (p1.empty())
    return 0;

  int collision = 0;
  Nodes p2;

  for (int i = 0; i < paths.size(); ++i)
  {
    if (a->getId() == i)
      continue;
    p2 = paths[i];
    if (p2.empty())
    continue;
      
    for (int t = 0; t < p1.size(); ++t)
    {
      if (t >= p2.size())
      {
        if (p1[t] == p2[p2.size() - 1])
        {
          ++collision;
          break;
        }
        continue;
      }
      if (p1[t] == p2[t])
      { // collision
        ++collision;
        break;
      }
      if (t > 0 && p1[t - 1] == p2[t] && p1[t] == p2[t - 1])
      {
        ++collision;
        break;
      }
      
    }
    
  }
  

  // error check
  if (collision > A.size() * (A.size() - 1) / 2)
  {
    std::cout << "error@ECBS::h3, invalid value, " << collision << "\n";
    std::exit(1);
  }

  return collision;
}

int SASB_ECBS::h3(Paths &paths)
{

  int collision = 0;
  Nodes p1, p2;

  for (int i = 0; i < paths.size(); ++i)
  {
    for (int j = i + 1; j < paths.size(); ++j)
    {
      if (paths[i].size() >= paths[j].size())
      {
        p1 = paths[i];
        p2 = paths[j];
      }
      else
      {
        p1 = paths[j];
        p2 = paths[i];
      }

      if (p2.empty())
        continue;

      for (int t = 0; t < p1.size(); ++t)
      {
        if (t >= p2.size())
        {
          // goal pos
          if (p1[t] == p2[p2.size() - 1])
          {
            ++collision;
            break;
          }
          continue;
        }
        if (p1[t] == p2[t])
        { // collision
          ++collision;
          break;
        }
        
        // intersection
        if (t > 0 && p1[t - 1] == p2[t] && p1[t] == p2[t - 1])
        {
          ++collision;
          break;
        }
      }
    }
  }

  // error check
  if (collision > A.size() * (A.size() - 1) / 2)
  {
    std::cout << "error@ECBS::h3, invalid value, " << collision << "\n";
    std::exit(1);
  }

  return collision;
}

struct Fib_FN
{ // Forcul Node for Fibonacci heap
  Fib_FN(AN *_node, int _h) : node(_node), h(_h) {}
  AN *node;
  int h;

  bool operator<(const Fib_FN &other) const
  {
    if (h != other.h)
    {
      return h > other.h;
    }
    else
    {
      if (node->f != other.node->f)
      {
        return node->f > other.node->f;
      }
      else
      {
        return node->g < other.node->g;
      }
    }
  }
};

Nodes SASB_ECBS::AstarSearch(Agent *a, CTNode *node)
{
  Constraint constraints = getConstraintsForAgent(node, a);

  Node *_s = a->getNode();
  Node *_g = a->getGoal();

  Nodes path, tmpPath, C; // return

  double bw = (*a).m_w;

  // ==== fast implementation ====
  // constraint free
  if (constraints.empty())
  {
    path = G->getPath(_s, _g);
    table_fmin.at(a->getId()) = path.size() - 1;
    return path;
  }

  // goal condition
  bool existGoalConstraint = false;
  int timeGoalConstraint = 0;
  for (auto c : constraints)
  {
    if (c->onNode && c->v == _g && c->t > timeGoalConstraint)
    {
      existGoalConstraint = true;
      timeGoalConstraint = c->t;
    }
  }
  // =============================

  auto paths = node->paths;

  int f, g;
  float ub;
  int ubori;
  std::string key, keyM, keyF;
  bool invalid = true;

  boost::heap::fibonacci_heap<Fib_AN> OPEN;
  std::unordered_map<std::string, boost::heap::fibonacci_heap<Fib_AN>::handle_type> SEARCHED;
  std::unordered_set<std::string> CLOSE; // key
  AN *n = new AN{_s, 0, pathDist(_s, _g), nullptr};
  auto handle = OPEN.push(Fib_AN(n));
  key = getKey(n);
  SEARCHED.emplace(key, handle);
  bool updateMin = true;
  std::unordered_map<std::string, int> table_conflict;
  table_conflict.emplace(key, 0);
  // FOCUL
  boost::heap::fibonacci_heap<Fib_FN> FOCUL;
  std::unordered_map<std::string,
                     boost::heap::fibonacci_heap<Fib_FN>::handle_type>
      SEARCHED_F;
  auto handle2 = FOCUL.push(Fib_FN(n, table_conflict.at(key)));

  //int stepCount = 0; // Counter for steps

  while (!OPEN.empty())
  {
    //stepCount++; // Increment step count

    if (updateMin || FOCUL.empty())
    {
      // argmin
      while (!OPEN.empty() && CLOSE.find(getKey(OPEN.top().node)) != CLOSE.end())
        OPEN.pop();
      if (OPEN.empty())
        break;
      n = OPEN.top().node;
      ubori = n->f;
      keyM = getKey(n);
      ub = n->f * bw;
      // update focul
      FOCUL.clear();
      SEARCHED_F.clear();
      for (auto itr = OPEN.ordered_begin(); itr != OPEN.ordered_end(); ++itr)
      {
        AN *l = (*itr).node;
        if ((float)l->f <= ub)
        {
          if (CLOSE.find(getKey(l)) == CLOSE.end())
          {
            key = getKey(l);
            auto handle_f = FOCUL.push(Fib_FN(l, table_conflict.at(key)));
            SEARCHED_F.emplace(key, handle_f);
          }
        }
        else
        {
          break;
        }
      }
    }

    // argmin in FOCUL
    n = FOCUL.top().node;
    key = getKey(n);
    FOCUL.pop();

    // already explored
    if (CLOSE.find(key) != CLOSE.end())
    {
      printf("Yes\n");
      continue;
    }

    // check goal
    if (n->v == _g)
    {
      if (!existGoalConstraint || timeGoalConstraint < n->g)
      {
        invalid = false;
        break;
      }
    }

    // update list
    updateMin = (key == keyM);
    CLOSE.emplace(key);

    // search neighbor
    C = G->neighbor(n->v);
    C.push_back(n->v);

    for (auto m : C)
    {
      g = n->g + 1;
      key = getKey(g, m);
      if (CLOSE.find(key) != CLOSE.end())
        continue;
      // check constraints
      auto constraint = std::find_if(constraints.begin(), constraints.end(),
                                     [a, g, m, n](Conflict *c)
                                     {
                                       if (c->a != a)
                                         return false;
                                       if (c->t != g)
                                         return false;
                                       if (c->onNode)
                                         return c->v == m;
                                       return c->v == m && c->u == n->v;
                                     });
      if (constraint != constraints.end())
      {
        continue;
      }
      f = g + pathDist(m, _g);

      // ==== fast implementation ====
      // when field is huge, this works well
      // if (existGoalConstraint) {
      //   f = pathDist(m, _g) + timeGoalConstraint;
      // }
      // =============================

      auto itrS = SEARCHED.find(key);
      AN *l;
      bool updateH = false;
      if (itrS == SEARCHED.end())
      {
        cnt++;
        l = new AN{m, g, f, n};
        auto handle = OPEN.push(Fib_AN(l));
        SEARCHED.emplace(key, handle);
        getPartialPath(l, tmpPath);
        table_conflict.emplace(key, h3(a, tmpPath, paths));
        //cnt2++;
      }
      else
      {
        auto handle = itrS->second;
        l = (*handle).node;
        if (l->f > f)
        {
          l->g = g;
          l->f = f;
          l->p = n;
          getPartialPath(l, tmpPath);
          table_conflict.at(key) = h3(a, tmpPath, paths);
          OPEN.increase(handle);
          updateH = true;
        }
        //cnt3 ++;
      }

      tmpPath.clear();
      if (f <= ub)
      {
        auto itrSF = SEARCHED_F.find(key);
        if (itrSF == SEARCHED_F.end())
        {
          auto handle_f = FOCUL.push(Fib_FN(l, table_conflict.at(key)));
          SEARCHED_F.emplace(key, handle_f);
        }
        else
        {
          if (updateH)
          {
            auto handle_f = itrSF->second;
            (*handle_f).h = table_conflict.at(key);
            FOCUL.increase(handle_f);
          }
        }
      }
    }
  }

  int soln = INT_MAX;

  // back tracking
  int fmin = 0;
  if (!invalid)
  { // check failed or not
    getPartialPath(n, path);
    fmin = ubori;
    //int trueStepCount = path.size(); 
    //std::cout << "Agent ID: " << (*a).getId() << "True Step Count: " << trueStepCount << std::endl; 
    //cout << "f_min: " << fmin << "\n";
  }
  else
  {
    node->valid = false;
    cout << "No solution found" << "\n";
  }
  table_fmin.at(a->getId()) = fmin;
  
  //int trueStepCount = path.size(); 
  //std::cout << "Agent ID: " << (*a).getId() << " True Step Count: " << trueStepCount << std::endl;
  
  return path;

  
}

void SASB_ECBS::getPartialPath(AN *n, Nodes &path)
{
  path.clear();
  AN *m = n;
  int count = 0;
  while (m != nullptr)
  {
    path.push_back(m->v);
    m = m->p;
    count++;
  }
  std::reverse(path.begin(), path.end());
}

std::string SASB_ECBS::logStr()
{
  std::string str;
  str += "[solver] type:SASB-ECBS\n";
  str += "[solver] w:" + std::to_string(w) + "\n";
  str += "[solver] ID:" + std::to_string(ID) + "\n";
  str += "[solver] Lowlevelnode:" + std::to_string(lowlevelnode) + "\n";
  str += "[solver] Highlevelnode:" + std::to_string(highLevelNode) + "\n";
  str += "[solver] ConflictCount:" + std::to_string(conflict_cnt) + "\n";

  str += Solver::logStr();

  std::cout << "[solver] type:SASB-ECBS" << std::endl;

  return str;
}
