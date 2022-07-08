import React from 'react'
import './gallery.css'
import ImageList from '@mui/material/ImageList';
import ImageListItem from '@mui/material/ImageListItem';

const itemData = [
    {
        img:'https://res.cloudinary.com/dlx2alkrl/image/upload/v1655828280/iecap/unknown_1_qot8nm.png',
        title: 'img1',
    },
    {
        img:'https://res.cloudinary.com/dlx2alkrl/image/upload/v1655828279/iecap/unknown_2_qeqmq9.png',
        title: 'img2',
    },
    {
        img:'https://res.cloudinary.com/dlx2alkrl/image/upload/v1655828279/iecap/unknown_3_zvxh2n.png',
        title: 'img3',
    },
    {
        img:'https://res.cloudinary.com/dlx2alkrl/image/upload/v1655828280/iecap/unknown_4_lahchj.png',
        title: 'img4',
    },
    {
        img:'https://res.cloudinary.com/dlx2alkrl/image/upload/v1655828279/iecap/unknown_vh7b98.png',
        title: 'img5',
    },

];

const Gallery = () => {
  return (
    <ImageList sx={{ width: '100%', height: 'auto' }} cols={2} >
    {itemData.map((item) => (
      <ImageListItem key={item.img}>
        <img
          src={`${item.img}`}
          srcSet={`${item.img}`}
          alt={item.title}
          loading="lazy"
        />
      </ImageListItem>
    ))}
  </ImageList>
  )
}

export default Gallery