/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author Stefan Jipa
 */
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.LinkedList;
import java.util.List;

public class Main {

    static String pattern = "^[a-zA-Z]*$";
    static int ok = 1;
    static int check_assert = 1;
    public static int returnValue = 0;
    public static String write_file;

    /* 
    * Evalueaza operatiile date si construieste rezultatul recursiv
     */
    public static int EvalOp(Node root, Database data) throws ScopeErr {
        if (root.value.compareTo("+") == 0) {

            return EvalOp(root.kids[0], data) + EvalOp(root.kids[1], data);
        }
        if (root.value.compareTo("*") == 0) {

            return EvalOp(root.kids[0], data) * EvalOp(root.kids[1], data);
        }
        if (root.value.compareTo("==") == 0) {

            return (EvalOp(root.kids[0], data) == EvalOp(root.kids[1], data)) ? 1 : 0;
        }
        if (root.value.compareTo("<") == 0) {

            return (EvalOp(root.kids[0], data) < EvalOp(root.kids[1], data)) ? 1 : 0;
        }
        if (root.value.matches(pattern)) {
            try {

                return data.valueOf(root.value);
            } catch (ScopeErr e) {
                ok = 0;
            }
        }
        return Integer.parseInt(root.value);
    }

    /* EvalOp program method
    * evalueaza recursiv programele si se foloseste de metoda EvalOp pt a
    * avalua fiecare operatie. Intoarce rezultatul final al programului.
     */
    public static int EvalProg(Node root, Database data) throws ScopeErr {
        if (root.value.compareTo("if") == 0) {
            try {  // pt if 
                data.isLast = false;
                // verific daca cond este adevarata
                if (EvalOp(root.kids[0], data) == 1) {
                    EvalProg(root.kids[1], data);  // da, atunci se eval 1 prog 
                } else {
                    EvalProg(root.kids[2], data);  // nu, se eval al 2-lea prog
                }
            } catch (ScopeErr e) {
                e.printStackTrace();
            }
        }
        if (root.value.compareTo("assert") == 0) {
            try {  // pt assert
                data.isLast = false;
                // verific daca cond este falsa
                if (EvalOp(root.kids[0], data) != 1) {
                    //da, pun flagul pe 0 pt a stii ca picat assert-ul
                    check_assert = 0;
                }
            } catch (ScopeErr e) {
                e.printStackTrace();
            }
        }
        if (root.value.compareTo(";") == 0) {
            data.isLast = false;
            // daca am ; apelez 2 subprograme
            EvalProg(root.kids[0], data);
            EvalProg(root.kids[1], data);
        }
        if (root.value.compareTo("=") == 0) {
            // daca primul element este un nume de variabila
            if (isAlpha(root.kids[0])) {
                try {
                    // adaug variabila si valoarea ei in hashmap
                    data.map.put(root.kids[0].value, EvalOp(root.kids[1], data));
                    data.isLast = false;
                } catch (ScopeErr e) {
                    e.printStackTrace();
                }
            }
        }
        if (root.value.compareTo("for") == 0) {  // for
            if (root.kids[0].value.compareTo("=") == 0) {
                EvalProg(root.kids[0], data);  // fac assigment
            }  // verific conditia de oprire a forului
            if (root.kids[1].value.compareTo("<") == 0
                    || root.kids[1].value.compareTo("==") == 0) {
                try {
                    // cat timp cond e valida
                    while (EvalOp(root.kids[1], data) == 1) {
                        EvalProg(root.kids[2], data);  // fac assigment
                        EvalProg(root.kids[3], data);  // exexut program
                    }
                    data.isLast = false;
                } catch (ScopeErr e) {
                    e.printStackTrace();
                }
            }

        }
        if (root.value.compareTo("return") == 0) {  // pt return
            try {
                // evaluez operatia
                returnValue = EvalOp(root.kids[0], data);
                data.isLast = true;  // am return
            } catch (ScopeErr e) {
                e.printStackTrace();
            }
        }
        // rezultatul intors
        return returnValue;
    }

    // verific daca este un nume de variabila
    public static boolean isAlpha(Node counter) {
        return counter.value.compareTo("return") != 0
                && counter.value.compareTo("for") != 0
                && counter.value.compareTo("if") != 0
                && counter.value.compareTo("assert") != 0
                && counter.value.matches(pattern);
    }

    /**
     * Metoda primeste ca parametru un string care reprezinta programul si
     * apeleaza metoda EvalProg care evalueaza programul dat. Aceasta intoarce
     * rezultatul programului.
     *
     * @param program un string care reprezinta programul
     * @return rezultatul programului
     */
    public static Integer GetResult(String program) {
        Node root = new Node(program);
        Database c = new Database();
        try {
            return EvalProg(root, c);
        } catch (ScopeErr e) {
            e.printStackTrace();
        }
        return ok;
    }

    /**
     * Metoda primeste ca parametru un string care reprezinta tot programul si
     * verifica daca are erori de scope, assert sau lipseste return. Daca apare
     * vreo eroare intoarce false, altfel true. Ordinea Scope > Missing return >
     * Assert failed
     *
     * @param program un string care reprezinta programul
     * @return true/false
     * @throws java.io.FileNotFoundException
     */
    public static Boolean ErrorCheck(String program) throws FileNotFoundException {
        int constant = 1;
        Database c = new Database();
        Node root = new Node(program), aux = null;
        int lastIndex = 0, count = 0;
        PrintWriter writer = new PrintWriter(write_file);
// incerc sa fac evaluarea programului
        try {
            EvalProg(root, c);

        } catch (NumberFormatException f) {
            ok = 0;
            constant = 0;
        } catch (ScopeErr e) {
            e.printStackTrace();
            ok = 0;
            constant = 0;
        }  // daca folosesc variabila nedeclarata/neinitializata
        if (constant == 0) {
            writer.print("Check failed");
            writer.close();
            return false;
        }
        if (c.isLast != true) {
            // daca nu am return
            writer.print("Missing return");
            writer.close();
            return false;
        }
        // daca am vreun assert picat
        if (check_assert == 0) {
            writer.print("Assert failed");
            writer.close();
            return false;
        }

        writer.close();
        return true;
    }

    /**
     * Param: Un string care contine o lista de programe(fiecare inclus in [])
     * Programele sunt separate prin spatii.
     *
     * @return : intoarce un vector de string-uri, fiecare element fiind un
     * program
     */
    public static String[] Listsplitting(String s) {
        String[] result = new String[0];
        List<String> l = new LinkedList<>();  // lista de string-uri
        int inside = 0;
        int start = 0, stop = 0;
        // parcurg string-ul primit
        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) == '[') {  // paranteze deschise
                inside++;
                stop++;
                continue;
            }
            if (s.charAt(i) == ']') {  // paranteze inchise
                inside--;
                stop++;
                continue;
            }  // daca nr de paranteze deschise = nr de paranteze inchise
            if (s.charAt(i) == ' ' && inside == 0) {  // si ese spatiu
                l.add(s.substring(start, stop));  // adaug 1 element in lista
                start = i + 1;  // dupa spatiu
                stop = start;
                continue;
            }
            stop++;
        }
        if (stop > start) {
            l.add(s.substring(start, stop));
        }
        // intorc lista de string-uri
        return l.toArray(new String[l.size()]);

    }

    public static void main(String[] args) throws IOException {
        /* Suggestion: use it for testing */
        String text = "";
        try (BufferedReader br = new BufferedReader(new FileReader(args[0]))) {
            String line;
            while ((line = br.readLine()) != null) {
                text += line;// process the line.
            }
        }
        write_file = args[1];
        text = text.replaceAll("\t", " ");  // inlocuiesc toate taburile cu space-uri
// daca imi raman mai multe space-uri la rand las doar unul singur
        text = text.replaceAll(" {2,}", " ");
        Node n = new Node(text);
// verific daca inputul are erori si daca nu are il evaluez
        if (ErrorCheck(text) == true) {
            Integer EvalOpdResult = Main.GetResult(text);
            try {
                PrintWriter writer = new PrintWriter(write_file);
                writer.print(EvalOpdResult);
                writer.close();
            } catch (IOException e) {
            }
        }
    }
}
