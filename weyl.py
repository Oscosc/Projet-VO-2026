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

    # Calcul principal (Équation 5 pour la 4ème composante)
    for x in range(rows):
        for y in range(cols):
            integral_images[1][x, y] = integral_images[0][x, cols-1] - integral_images[0][x, y]
            integral_images[2][x, y] = integral_images[0][rows-1, y] - integral_images[0][x, y]
            # On vire ça de la boucle integral_images[0][rows-1, cols-1] ici :
            integral_images[3][x, y] = integral_images[0][x, y] - integral_images[0][rows-1, y] - integral_images[0][x, cols-1]

    # Correcting specific cases (Équations 2, 3 et 4)
    integral_images[1][:, cols-1] = integral_images[0][:, cols-1]                                           
    integral_images[2][rows-1, :] = integral_images[0][rows-1, :]                                           
    
    # On fait la composante 4 ici avec ses cas speciaux.
    integral_images[3][rows-1, :-1] = -integral_images[0][rows-1, :-1]  # Π4 : if x = W, y != H
    integral_images[3][:-1, cols-1] = -integral_images[0][:-1, cols-1]  # Π4 : if x != W, y = H
    integral_images[3][rows-1, cols-1] = 0                              # Π4 : if x = W, y = H

    minimums = np.minimum(0, np.min(integral_images, axis=(1, 2)))
    maximums = np.maximum(0, np.max(integral_images, axis=(1, 2)))
    
    return np.max(maximums - minimums)

def PatternMatching(pattern, image_in, WeylsFunction=WeylOptimized,
                    display_execution_time=True,
                    display_progress_bar=True):
    """ Cherche la position la plus probable pour un pattern dans une image.

    Args:
        pattern: pattern recherché dans l'image
        image_in: image dans laquelle on cherche le pattern
        WeylsFunction: Fonction a utiliser pour le calcul de la disparité de Weyl
        (par défaut WeylOptimized)
        display_execution_time: affiche des informations sur les temps d'exécution du matching
        (par défaut True)
    
    Returns:
        image_out: carte des disparités de Weyl dans l'image, le point où se trouve le minimum
        est le point le plus probable de position du pattern.
        position: position dans la carte des disparités où la disparité est la plus faible
    """

    nx, ny = image_in.shape
    px, py = pattern.shape
    padx, pady = px // 2, py // 2

    # Execution time section
    start_time = time.time()
    
    image_w_borders = cv2.copyMakeBorder(image_in, pady, pady, padx, padx, cv2.BORDER_CONSTANT, value=0.0)
    image_out = np.zeros((nx, ny))
    
    for i in tqdm(range(nx), disable=not display_progress_bar):
        for j in range(ny):
            zone = image_w_borders[i:i+px, j:j+py]
            diff = zone - pattern
            image_out[i, j] = WeylsFunction(diff)
    
    position = np.unravel_index(image_out.argmin(), image_out.shape)


    if(display_execution_time):
        total_time = time.time() - start_time
        print("------------------------------------------------------------------")
        print(f"Total execution time        : {total_time} seconds")
        print(f"Average row execution time  : {total_time / nx} seconds")
        print(f"Average cell execution time : {total_time / (nx * ny)} seconds")
        print("------------------------------------------------------------------")
    
    return image_out, position


def Disparity(img_left, img_right, patch_size=9, max_disparity=60, compute_right=False):
    """ Calcule la carte de disparité de l'image A par rapport à l'image B.
    Le calcul est effectué de façon vectoriel pour optimiser le temps d'exécution
    (plutôt que de réutiliser la fonction WeylOptimized dans une boucle)

    Args:
        img_A: image de reférence pour le calcul de la carte de disparité
        img_B: image de comparaison pour le calcul de la carte de disparité
        patch_size: taille du patch de reconnaissance (default 9)
        max_disparity: disparité maximale possible entre deux pixels, pour
        accélérer la recherche (default 60)
    
    Returns:
        Carte des disparités de l'image par la méthode de Weyl
    """

    Rfactor = 1.0
    R2factor = 1
    if compute_right :
        Rfactor = -1.0
        R2factor = 0

    H, W = img_left.shape
    K = patch_size
    padding = K // 2
    
    best_disparity = np.zeros((H, W), dtype=np.float32)
    min_costs = np.full((H, W), np.inf, dtype=np.float32)
    
    for d in tqdm(range(max_disparity)):
        
        if d == 0:
            diff_img = Rfactor*img_left - Rfactor*img_right
        else:
            diff_img = Rfactor*img_left[:, d:] - Rfactor*img_right[:, :-d]
            
        I = cv2.integral(diff_img, sdepth=cv2.CV_32F)
        
        valid_H = H - K + 1
        valid_W = W - d - K + 1
        
        if valid_W <= 0:
            break
            
        min_c1 = np.zeros((valid_H, valid_W), dtype=np.float32)
        max_c1 = np.zeros((valid_H, valid_W), dtype=np.float32)
        min_c2 = np.zeros((valid_H, valid_W), dtype=np.float32)
        max_c2 = np.zeros((valid_H, valid_W), dtype=np.float32)
        min_c3 = np.zeros((valid_H, valid_W), dtype=np.float32)
        max_c3 = np.zeros((valid_H, valid_W), dtype=np.float32)
        min_c4 = np.zeros((valid_H, valid_W), dtype=np.float32)
        max_c4 = np.zeros((valid_H, valid_W), dtype=np.float32)
        
        for u in range(1, K + 1):
            for v in range(1, K + 1):
                
                s1 = (I[u:valid_H+u, v:valid_W+v] - I[0:valid_H, v:valid_W+v] - 
                      I[u:valid_H+u, 0:valid_W] + I[0:valid_H, 0:valid_W])
                np.minimum(min_c1, s1, out=min_c1)
                np.maximum(max_c1, s1, out=max_c1)
                
                s2 = (I[u:valid_H+u, K:valid_W+K] - I[0:valid_H, K:valid_W+K] - 
                      I[u:valid_H+u, K-v:valid_W+K-v] + I[0:valid_H, K-v:valid_W+K-v])
                np.minimum(min_c2, s2, out=min_c2)
                np.maximum(max_c2, s2, out=max_c2)
                
                s3 = (I[K:valid_H+K, v:valid_W+v] - I[K-u:valid_H+K-u, v:valid_W+v] - 
                      I[K:valid_H+K, 0:valid_W] + I[K-u:valid_H+K-u, 0:valid_W])
                np.minimum(min_c3, s3, out=min_c3)
                np.maximum(max_c3, s3, out=max_c3)
                
                s4 = (I[K:valid_H+K, K:valid_W+K] - I[K-u:valid_H+K-u, K:valid_W+K] - 
                      I[K:valid_H+K, K-v:valid_W+K-v] + I[K-u:valid_H+K-u, K-v:valid_W+K-v])
                np.minimum(min_c4, s4, out=min_c4)
                np.maximum(max_c4, s4, out=max_c4)
        
        weyl_scores = np.maximum.reduce([
            max_c1 - min_c1,
            max_c2 - min_c2,
            max_c3 - min_c3,
            max_c4 - min_c4
        ])
        
        target_min_costs = min_costs[padding:H-padding, padding+(d*R2factor):padding+(d*R2factor)+valid_W]
        
        mask = weyl_scores < target_min_costs
        
        target_min_costs[mask] = weyl_scores[mask]
        min_costs[padding:H-padding, padding+(d*R2factor):padding+(d*R2factor)+valid_W] = target_min_costs
        
        target_best_disp = best_disparity[padding:H-padding, padding+(d*R2factor):padding+(d*R2factor)+valid_W]
        target_best_disp[mask] = d
        best_disparity[padding:H-padding, padding+(d*R2factor):padding+(d*R2factor)+valid_W] = target_best_disp

    return best_disparity #[padding:-padding, padding:-padding] = bords noirs mais même taille d'image finale