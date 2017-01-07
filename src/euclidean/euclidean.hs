#!/usr/bin/env runhaskell

-- Generate data.h and data.c

import           Data.List     (intercalate, partition, unfoldr)

-- http://unthingable.eat-up.org/tag/music.html

-- | Euclidean rhythm generator
euclidean :: Int    -- ^ fill steps
          -> Int    -- ^ total steps
          -> [Bool]
euclidean k n = concat . efold $
                replicate k [True] ++ replicate (n - k) [False]

-- | A hybrid of zip and outer join, takes two lists of different lengths and
-- concatenates their elements pairwise
ezip :: [[a]] -> [[a]] -> [[a]]
ezip x [] = x
ezip [] x = x
ezip (x:xs) (y:ys) = (x ++ y) : ezip xs ys

-- | Repeatedly applies the tail end of the sequence under construction to the
-- head, using ezip, until either there are 3 or fewer subpatterns or the
-- pattern is cyclic.
efold :: Eq a => [[a]] -> [[a]]
efold xs
  | length xs <= 3 = xs
  | null a         = xs
  | otherwise      = efold $ ezip a b
  where (a, b) = partition (/= last xs) xs

-- | All the Euclidean rhythms of a given length
allEuclideans :: Int -> [[Bool]]
allEuclideans n = fmap (`euclidean` n) [0..n]

-- | Pad a list of `Bool` with False out to length `n`
pad :: Int -> [Bool] -> [Bool]
pad n xs
  | length xs >= n = xs
  | otherwise      = pad n $ xs ++ [False]

-- | Chunking algorithm
chunk :: Int -> [a] -> [[a]]
chunk n = takeWhile (not.null) . unfoldr (Just . splitAt n)

-- | Split a list of bools into a list of list of bools, each padded to 8 bits
-- (e.g char)
bitArray :: [Bool] -> [[Bool]]
bitArray xs = pad 8 <$> chunk 8 xs

bitArrayCLiteral :: [[[Bool]]] -> String
bitArrayCLiteral xs = multiLineCArray $ oneLineCArray <$> (fmap . fmap) bitCLiteral xs
  where oneLineCArray :: [String] -> String
        oneLineCArray ys = "{" ++ intercalate ", " ys ++ "}"
        multiLineCArray :: [String] -> String
        multiLineCArray ys = "{\n" ++ intercalate ",\n" (fmap ("\t"++) ys) ++ "\n}"
        bitCLiteral :: [Bool] -> String
        bitCLiteral ys = "0b" ++ fmap lit ys
          where lit True = '1'
                lit False = '0'

bytes :: Int -> Int
bytes x = x `div` 8 + p
  where p = if x `rem` 8 > 0 then 1 else 0

arrayPrototype :: Int -> String
arrayPrototype x = "const char table_euclidean_" ++ show x ++ "[" ++ show (x + 1) ++ "][" ++ show (bytes x) ++ "]"

array :: Int -> String
array x = arrayPrototype x ++ " = " ++ bitArrayCLiteral (bitArray <$> allEuclideans x) ++ ";"

dataC :: String
dataC = "// clang-format off\n\n" ++
        intercalate "\n\n" (fmap array [1..32]) ++ "\n"

headerC :: String
headerC = "// clang-format off\n\n" ++
          "#ifndef _EUCLIDEAN_DATA_H_\n" ++
          "#define _EUCLIDEAN_DATA_H_\n" ++
          concatMap (\i -> "extern " ++ arrayPrototype i ++ ";\n") [1..32] ++
          "#endif\n"

main :: IO ()
main = do writeFile "data.h" headerC
          writeFile "data.c" dataC
