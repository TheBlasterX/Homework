module Interpreter
  (
    -- * Types
    Prog,
    Asgn,

    -- * Functions
    evalRaw,
    evalAdt,
  ) where

-------------------------------------------------------------------------------
--------------------------------- The Expr ADT  -------------------------------
-------------------------------------------------------------------------------
data Expr = Add Expr Expr
          | Sub Expr Expr
          | Mult Expr Expr
          | Equal Expr Expr
          | Smaller Expr Expr
          | Symbol String
          | Value Int deriving (Show, Read)

-- TODO Implement a parser for the Expr ADT.
--

-------------------------------------------------------------------------------
---------------------------------- The Prog ADT -------------------------------
-------------------------------------------------------------------------------
data Asgn = Asgn String Expr deriving (Show, Read)

data Prog = Eq Asgn
          | Seq Prog Prog
          | If Expr Prog Prog
          | For Asgn Expr Asgn Prog
          | Assert Expr
          | Return Expr deriving (Show, Read)

-- TODO Implement a parser for the Prog ADT.
--

-- TODO The *parse* function. It receives a String - the program in
-- a "raw" format and it could return *Just* a program as an instance of the
-- *Prog* data type if no parsing errors are encountered, or Nothing if parsing
-- failed.
--
-- This is composed with evalAdt to yield the evalRaw function.
parse :: String -> Maybe Prog
parse = \_ -> Nothing

-------------------------------------------------------------------------------
-------------------------------- The Interpreter ------------------------------
-------------------------------------------------------------------------------

-- TODO The *evalAdt* function.  It receives the program as an instance of the
-- *Prog* data type and returns an instance of *Either String Int*; that is,
-- the result of interpreting the program.
--
-- The result of a correct program is always an Int.  This is a simplification
-- we make in order to ease the implementation.  However, we wrap this Int with
-- a *Either String* type constructor to handle errors.  The *Either* type
-- constructor is defined as:
--
-- data Either a b = Left a | Right b
--
-- and it is generally used for error handling.  That means that a value of
-- *Left a* - Left String in our case - wraps an error while a value of *Right
-- b* - Right Int in our case - wraps a correct result (notice that Right is a
-- synonym for "correct" in English).
-- 
-- For further information on Either, see the references in the statement of
-- the assignment.
--

-- Baza de date, lista, cu elemente pair (String, Int)
type Database a = [(String, Int)] 
-- Obtin element din baza de date
getvalue :: String -> (Database a) -> (Either String Int) 
getvalue name [] = Left "ERROR"
getvalue name ((s, val) : data_base) = if (s == name)
              then Right (val)
              else getvalue name data_base

-- Adaug element in baza de date
putvalue :: Database a -> String -> Int -> Database a
putvalue [] str value = (str, value) : []
putvalue ((str1, value') : context) str value = if(str == str1)
                                          then ((str1, value) : context)
                                          else ((str1, value') : (putvalue context str value))

-- Obtin un Int dintr-un Either String Int
getINT :: Either String Int -> Int
getINT (Right x) = x

-- evalOP function for expressions using Pattern Matching
evalOP :: Expr -> Database a -> Either String Int

-- value
evalOP (Value a) context = Right a
-- symbol
evalOP (Symbol a) context = getvalue a context
-- comparation
evalOP (Smaller a b) context = case (evalOP a context , evalOP b context) of
    (Right a, Right b) -> if (a < b) then (Right 1) else (Right 0)
    _ -> Left "ERROR"
-- equality
evalOP (Equal a b) context = case (evalOP a context , evalOP b context) of
    (Right a, Right b) -> if (b == a) then (Right 1) else (Right 0)
    _ -> Left "ERROR"
-- addition
evalOP (Add a b) context = case (evalOP a context, evalOP b context) of
    (Right a, Right b) -> Right(a + b)
    _ -> Left "ERROR"
-- substraction
evalOP (Sub a b) context = case (evalOP a context, evalOP b context) of
    (Right a, Right b) -> Right (a - b)
    _ -> Left "ERROR"
-- multiplication
evalOP (Mult a b) context = case (evalOP a context, evalOP b context) of
    (Right a, Right b) -> Right (a * b)
    _ -> Left "ERROR"

-- Evaluarea programului
evalPR :: Prog -> Database a -> (Database a , Either String Int)

-- Pt Eq (asignez vaariabila)
evalPR (Eq (Asgn s ex)) context =
  if ((evalOP ex context) == Left "ERROR") then (context, Left "Uninitialized variable")
                                          else (putvalue context s (getINT (evalOP ex context)), Right 0)

-- Pt Seq (evaluez 2 programe)
evalPR (Seq prog1 prog2) context =
  case (evalPR prog1 context) of
    (context', Right (-1)) -> (context', Right(-1))  
    (context', Left _) -> (context', Left "Uninitialized variable")  
    (context', Right val) -> evalPR prog2 context'                 
                                                                         
-- Pt If (verific expr, dc e adevarata evaluez prog1, altfel evaluez prog2)
evalPR (If expr progr1 progr2) context =
  case (evalOP expr context) of
  Right (-1) -> (context, Right (-1))  -- verific daca am vreun assert picat dinainte
  _ -> if ((Right 1) == (evalOP expr context)) -- verific expresia
    then evalPR progr1 context
    else if ((Left "ERROR") == (evalOP expr context))
          then (context, Left "Uninitialized variable")
    else evalPR progr2 context

-- Pt For (asigneaza val de start a contorului si apeleaza o alta functie)
evalPR (For (Asgn s1 ex1) expr (Asgn s2 ex2) prog) context = 
  let (context', rez) = (evalPR (Eq (Asgn s1 ex1)) context)
  in case (evalOP expr context') of
    Left "ERROR" -> (context', rez)
    _ -> repeatFor expr (Eq (Asgn s2 ex2)) prog context' 

-- Pt Assert, intorc Right (-1) pt assert picat in functia recursiva
evalPR (Assert expr) context =
  if ((Right 1) == (evalOP expr context))
    then (context, Right 0)
  else (context, Right (-1))

-- Pt Return
-- intorc Right (-1) pt assert picat
-- evalPR intoarce by default Right 0 daca programul nu are return
-- de aceea daca intorc o valoare, defapt intorc Right (val + 1), val >=0
-- astfel ma asigur ca daca programul trebuie sa intoarca 0, sa nu afisez
-- Missing return
evalPR (Return expr) context =
  case (evalOP expr context) of
    Right (-1) -> (context, Right (-1))
    Right val -> (context, Right (val + 1))
    _ -> (context, Left "Uninitialized variable")

-- Pt For: repeta corpul for-ului cat timp cond e adev si increm contorul
repeatFor :: Expr -> Prog -> Prog ->  Database a -> (Database a, Either String Int)
repeatFor expr  (Eq (Asgn s ex)) prog context =
  if ((Right 1) == (evalOP expr context)) then
    let (context', rez) = (evalPR prog context)
        (context'', rez') = (evalPR (Eq (Asgn s ex)) context')
    in case (rez) of
      Right (-1) -> (context', Right (-1))
      Left "ERROR" -> (context', Left "ERROR")
      _ -> repeatFor expr (Eq (Asgn s ex)) prog context''
  else (context , Right 0)

-- Rezultatul evaluarii
-- Daca primesc Right (-1) inseamna ca a picat cel putin un Assert
-- Daca primsc Right 0 inseamna ca programul nu are return
-- Daca primesc Right val scriu Right (val - 1) pt ca evalPR intoarce Right (val + 1)
-- Pt a putea trata separat Right 0 => Missing return
-- Ultimul caz este pt Left "Uninitialized variable")
evaluate :: (Database a, Either String Int) -> (Either String Int)
evaluate (_, Right 0) = Left "Missing return"
evaluate (_, Right (-1)) = Left "Assert failed"  
evaluate (_, Right val) = Right (val - 1)
evaluate (_, a) = a

evalAdt :: Prog -> Either String Int
evalAdt p = evaluate (evalPR p [])

-- The *evalRaw* function is already implemented, but it relies on the *parse*
-- function which you have to implement.
--
-- Of couse, you can change this definition.  Only its name and type are
-- important.
evalRaw :: String -> Either String Int
evalRaw rawProg = case parse rawProg of
                    Just prog -> evalAdt prog
                    Nothing   -> Left "Syntax error"