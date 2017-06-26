/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author Stefan Jipa
 */
import java.util.HashMap;

public class Database {

    public boolean isLast = false;
    public HashMap<String, Integer> map = new HashMap<String, Integer>();
    // adauga un element in hashmap
    public void add(String v, Integer i) {
        map.put(v, i);
    }

    public Integer valueOf(String v) throws ScopeErr {
        // daca variabila folosita este initializata
        if (map.containsKey(v)) {
            return map.get(v);  // intoarce valoarea ei
        } else {
            throw new ScopeErr();
        }

    }
}
