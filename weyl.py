import numpy as np
import time
import cv2
from tqdm import tqdm



def IntegralImageNaive(image_in) -> int:
    """Calcule l'intégrale d'une image de manière naïve.
    Cette fonction est appelée par WeylNaive().
    
    Args:
        image_in: image dont on veut calculer l'intégrale naïve
    
    Returns:
        int: intégrale naïve de l'image, soit max - min de l'intégrale
    """

    nx, ny = image_in.shape

    max_value, min_value = 0, 0
    for k in range(1, nx+1):
        for l in range(1, ny+1):
            value = np.sum(image_in[:k, :l])
            if(value > max_value):   max_value = value
            elif(value < min_value): min_value = value
    final_value = max_value - min_value

    return final_value


def WeylNaive(image_in) -> int:
    """Calcule la valeur de discrépance de Weyl de manière naïve.
    
    Args:
        image_in: différence entre deux image dont on veut calculer la discrépance
    
    Returns:
        int: valeur de discrépance de Weyl entre les deux images
    """
    
    top_left_corner = IntegralImageNaive(image_in)
    top_rght_corner = IntegralImageNaive(np.fliplr(image_in))
    bot_left_corner = IntegralImageNaive(np.flipud(image_in))
    bot_rght_corner = IntegralImageNaive(np.fliplr(np.flipud(image_in)))

    return np.max([top_left_corner, top_rght_corner, bot_left_corner, bot_rght_corner])


def IntegralImageOptimized(image_in):
    """Calcule l'intégrale d'une image de manière optimisée.
    Cette fonction est appelée par WeylOptimized().
    
    Args:
        image_in: image dont on veut calculer l'intégrale optimisée
    
    Returns:
        int: intégrale optimisée de l'image
    """
    rows, cols = image_in.shape
    integral_image = np.zeros((rows, cols))

    # First cell
    integral_image[0, 0] = image_in[0, 0]

    # First column
    for x in range(1, rows):
        integral_image[x, 0] = integral_image[x-1, 0] + image_in[x, 0]

    # Others cells
    for y in range(1, cols):
        accumulator = image_in[0, y]
        integral_image[0, y] = integral_image[0, y-1] + accumulator
        for x in range(1, rows):
            accumulator += image_in[x, y]
            integral_image[x, y] = integral_image[x, y-1] + accumulator
    
    return integral_image


def WeylOptimized(image_in):
    """Calcule la valeur de discrépance de Weyl de manière optimisée selon
    l'article : On a Fast Implementation of a 2D-Variant of Weyl’s Discrepancy Measure.
    
    Args:
        image_in: différence entre deux image dont on veut calculer la discrépance
    
    Returns:
        int: valeur de discrépance de Weyl entre les deux images
    """

    rows, cols = image_in.shape
    integral_images = np.zeros((4, rows, cols))
    integral_images[0] = IntegralImageOptimized(image_in)

    for x in range(rows):
        for y in range(cols):
            integral_images[1][x, y] = integral_images[0][x, cols-1] - integral_images[0][x, y]
            integral_images[2][x, y] = integral_images[0][rows-1, y] - integral_images[0][x, y]
            integral_images[3][x, y] = integral_images[0][rows-1, cols-1] + integral_images[0][x, y] - integral_images[0][rows-1, y] - integral_images[0][x, cols-1]

    # Correcting specific cases
    integral_images[1][:, cols-1] = integral_images[0][:, cols-1]                                           # Π3 : if y = H
    integral_images[2][rows-1, :] = integral_images[0][rows-1, :]                                           # Π3 : if y = H
    integral_images[3][rows-1, :-1] = integral_images[0][rows-1, cols-1] - integral_images[0][rows-1, :-1]  # Π4 : if x = W, y != H
    integral_images[3][:-1, cols-1] = integral_images[0][rows-1, cols-1] - integral_images[0][:-1, cols-1]  # Π4 : if x != W, y = H
    integral_images[3][rows-1, cols-1] = integral_images[0][rows-1, cols-1]                                 # Π4 : if x = W, y = H

    minimums = np.minimum(0, np.min(integral_images, axis=(1, 2)))
    maximums = np.maximum(0, np.max(integral_images, axis=(1, 2)))
    
    return np.max(maximums - minimums)


def PatternMatching(pattern, image_in, WeylsFunction=WeylOptimized, display_execution_time=True):
    """ Cherche la position la plus probable pour un pattern dans une image.

    Args:
        pattern: pattern recherché dans l'image
        image_in: image dans laquelle on cherche le pattern
        WeylsFunction: Fonction a utiliser pour le calcul de la disparité de Weyl
        (par défaut WeylOptimized)
        display_execution_time: affiche des informations sur les temps d'exécution du matching
        (par défaut True)
    
    Returns:
        carte des disparités de Weyl dans l'image, le point où se trouve le minimum
        est le point le plus probable de position du pattern.
    """

    nx, ny = image_in.shape
    px, py = pattern.shape
    padx, pady = px // 2, py // 2

    # Execution time section
    start_time = time.time()
    
    image_w_borders = cv2.copyMakeBorder(image_in, pady, pady, padx, padx, cv2.BORDER_CONSTANT, value=0.0)
    image_out = np.zeros((nx, ny))
    
    for i in tqdm(range(nx)):
        for j in range(ny):
            zone = image_w_borders[i:i+px, j:j+py]
            diff = zone - pattern
            image_out[i, j] = WeylsFunction(diff)


    if(display_execution_time):
        total_time = time.time() - start_time
        print("------------------------------------------------------------------")
        print(f"Total execution time        : {total_time} seconds")
        print(f"Average row execution time  : {total_time / nx} seconds")
        print(f"Average cell execution time : {total_time / (nx * ny)} seconds")
        print("------------------------------------------------------------------")
    
    return image_out