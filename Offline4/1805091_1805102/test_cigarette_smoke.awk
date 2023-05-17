BEGIN {
    i=0
    isSupply = 0
    for (i = 0; i < MAX; i++) {
	agent[i] = 0
	smocker[i] = 0
    }
    i=0
}
  {
    if ($1 == "Agent") {
        if($3== "tobacco" && $5 =="paper"){
            agent[i] = 1
            i=i+1
        }
        else if($3== "tobacco" && $5 =="match"){
            agent[i] = 2
            i=i+1
        }
        else if($3== "paper" && $5 =="match"){
            agent[i] = 3
            i=i+1
        }
        isSupply=1
    }
    if($1 == "Smoker" && isSupply == 1){
        if($3== "tobacco"  && agent[i-1] == 3){
            smocker[i-1] = 1
        }
        else if($3== "paper" && agent[i-1] == 2){
            smocker[i-1] = 1
        }
        else if($3== "match" && agent[i-1] == 1){
            smocker[i-1] = 1
        }
        isSupply=0
    }
  }  
END {
    total=0
    for (i = 0; i < MAX; i++) {
         total += smocker[i]
    } 
    if(MAX== total){
        print("passed\n")
    }
    else{
        printf("%d/%d test case passed\n", total, MAX)
    }
}